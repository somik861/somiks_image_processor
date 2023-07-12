#include "png.hpp"
#include "common_macro.hpp"
#include <map>
#include <png.h>
#include <zlib.h>

namespace {
std::string format_to_string(png_uint_32 format) {
	switch (format) {
	case PNG_FORMAT_GRAY:
		return "GRAY";
	case PNG_FORMAT_GA:
		return "GRAY (with alpha)";
	case PNG_FORMAT_AG:
		return "GRAY (with alpha first)";
	case PNG_FORMAT_RGB:
		return "RGB";
	case PNG_FORMAT_BGR:
		return "BGR";
	case PNG_FORMAT_RGBA:
		return "RGBA";
	case PNG_FORMAT_ARGB:
		return "ARGN";
	case PNG_FORMAT_BGRA:
		return "BGRA";
	case PNG_FORMAT_ABGR:
		return "ABGR";
	case PNG_FORMAT_LINEAR_Y:
		return "Linear GRAY";
	case PNG_FORMAT_LINEAR_Y_ALPHA:
		return "Linear GRAY (with alpha)";
	case PNG_FORMAT_LINEAR_RGB:
		return "Linear RGB";
	case PNG_FORMAT_LINEAR_RGB_ALPHA:
		return "Linear RGBA";
	case PNG_FORMAT_RGB_COLORMAP:
		return "RGB with Colormap";
	case PNG_FORMAT_BGR_COLORMAP:
		return "BGR with Colormap";
	case PNG_FORMAT_RGBA_COLORMAP:
		return "RGB with Colormap";
	case PNG_FORMAT_ARGB_COLORMAP:
		return "ARGB with Colormap";
	case PNG_FORMAT_BGRA_COLORMAP:
		return "BGRA with Colormap";
	case PNG_FORMAT_ABGR_COLORMAP:
		return "ABGR with Colormap";
	}

	return std::format("Unrecognized format ID, ID: {}", format);
}
} // namespace

namespace ssimp::formats {

/* static */ bool PNG::image_count_supported(std::size_t count) {
	return count == 1;
}

/* static */ bool PNG::image_dims_supported(std::span<const std::size_t> dims) {
	return dims.size() == 2 && dims[0] > 0 && dims[1] > 0;
}

/* static */ std::optional<std::vector<img::LocalizedImage>>
PNG::load_image(const std::filesystem::path& path,
                const option_types::options_t& options) {
	std::string str_path = path.string();

	png_image image;
	std::memset(&image, 0, sizeof(png_image));
	image.version = PNG_IMAGE_VERSION;

	if (!png_image_begin_read_from_file(&image, str_path.c_str()))
		return {};

	png_bytep buffer = nullptr;
	std::vector<img::LocalizedImage> out;

	png_uint_32 mem_format;
	switch (image.format) {
	case PNG_FORMAT_GRAY:
	case PNG_FORMAT_LINEAR_Y:
		mem_format = PNG_FORMAT_GRAY;
		break;
	case PNG_FORMAT_GA:
	case PNG_FORMAT_AG:
	case PNG_FORMAT_LINEAR_Y_ALPHA:
		mem_format = PNG_FORMAT_GA;
		break;
	case PNG_FORMAT_RGB:
	case PNG_FORMAT_BGR:
	case PNG_FORMAT_LINEAR_RGB:
	case PNG_FORMAT_RGB_COLORMAP:
	case PNG_FORMAT_BGR_COLORMAP:
		mem_format = PNG_FORMAT_RGB;
		break;
	case PNG_FORMAT_RGBA:
	case PNG_FORMAT_ARGB:
	case PNG_FORMAT_BGRA:
	case PNG_FORMAT_ABGR:
	case PNG_FORMAT_LINEAR_RGB_ALPHA:
	case PNG_FORMAT_RGBA_COLORMAP:
	case PNG_FORMAT_ARGB_COLORMAP:
	case PNG_FORMAT_BGRA_COLORMAP:
	case PNG_FORMAT_ABGR_COLORMAP:
		mem_format = PNG_FORMAT_RGBA;
		break;
	default:
		throw ssimp::exceptions::Unsupported(
		    std::format("Unsupported input format of ID: {}", image.format));
	}

	std::array dims{std::size_t(image.width), std::size_t(image.height)};
	if (mem_format == PNG_FORMAT_RGBA) {
		img::ndImage<img::RGBA8> typed_img(dims);
		out.emplace_back(typed_img);
		buffer = reinterpret_cast<png_bytep>(typed_img.span().data());
	} else if (mem_format == PNG_FORMAT_RGB) {
		img::ndImage<img::RGB8> typed_img(dims);
		out.emplace_back(typed_img);
		buffer = reinterpret_cast<png_bytep>(typed_img.span().data());
	} else if (mem_format == PNG_FORMAT_GRAY) {
		img::ndImage<img::GRAY8> typed_img(dims);
		out.emplace_back(typed_img);
		buffer = reinterpret_cast<png_bytep>(typed_img.span().data());
	} else if (mem_format == PNG_FORMAT_GA) {
		img::ndImage<img::GRAY8A> typed_img(dims);
		out.emplace_back(typed_img);
		buffer = reinterpret_cast<png_bytep>(typed_img.span().data());
	}
	assert(buffer != nullptr);

	png_image_finish_read(&image, nullptr, buffer, 0, nullptr);

	return out;
}

/* static */ std::optional<ImageProperties>
PNG::get_information(const std::filesystem::path& path,
                     const option_types::options_t& options) {
	std::string str_path = path.string();
	ImageProperties props{name};

	// initialization
	png_image image;
	std::memset(&image, 0, sizeof(png_image));
	image.version = PNG_IMAGE_VERSION;

	if (!png_image_begin_read_from_file(&image, str_path.c_str()))
		return {};

	props.dims = {image.width, image.height};
	props.others["Colorspace"] = format_to_string(image.format);

	png_image_free(&image);

	return props;
}

template <typename T>
    requires mt::traits::is_any_of_tuple_v<T, PNG::supported_types>
/* static */ void PNG::save_image(const std::vector<img::ndImage<T>>& imgs,
                                  const std::filesystem::path& path,
                                  const option_types::options_t& options) {

	std::string str_path = path.string();
	const auto& img_ = imgs[0];

	// initialization
	png_image image;
	std::memset(&image, 0, sizeof(png_image));

	image.width = png_uint_32(img_.dims()[0]);
	image.height = png_uint_32(img_.dims()[1]);
	image.version = PNG_IMAGE_VERSION;
	if constexpr (std::is_same_v<T, img::GRAY8>)
		image.format = PNG_FORMAT_GRAY;
	else if constexpr (std::is_same_v<T, img::RGB8>)
		image.format = PNG_FORMAT_RGB;
	else if constexpr (std::is_same_v<T, img::RGBA8>)
		image.format = PNG_FORMAT_RGBA;

	if (std::get<bool>(options.at("fast_save")))
		image.flags |= PNG_IMAGE_FLAG_FAST;

	if (!png_image_write_to_file(&image, str_path.c_str(), 0,
	                             img_.span().data(), 0, nullptr))
		throw ssimp::exceptions::IOError(
		    std::format("Could not write: {}", str_path));
}

INSTANTIATE_SAVE_TEMPLATE(PNG, img::GRAY8);
INSTANTIATE_SAVE_TEMPLATE(PNG, img::GRAY8A);
INSTANTIATE_SAVE_TEMPLATE(PNG, img::RGB8);
INSTANTIATE_SAVE_TEMPLATE(PNG, img::RGBA8);

} // namespace ssimp::formats
