#include "png.hpp"
#include "common_macro.hpp"
#include <map>
#include <png.h>
#include <zlib.h>

namespace {
template <typename key_t, typename value_t>
std::unordered_map<value_t, key_t>
_mirror_map(const std::unordered_map<key_t, value_t>& map) {
	std::unordered_map<value_t, key_t> out;
	for (const auto& [k, v] : map)
		out[v] = k;
	return out;
}

std::unordered_map<png_uint_32, std::string> _format_string_map{
    {PNG_FORMAT_GRAY, "GRAY"},
    {PNG_FORMAT_GA, "GRAY (with alpha)"},
    {PNG_FORMAT_AG, "GRAY (with alpha first)"},
    {PNG_FORMAT_RGB, "RGB"},
    {PNG_FORMAT_BGR, "BGR"},
    {PNG_FORMAT_RGBA, "RGBA"},
    {PNG_FORMAT_ARGB, "ARGN"},
    {PNG_FORMAT_BGRA, "BGRA"},
    {PNG_FORMAT_ABGR, "ABGR"},
    {PNG_FORMAT_LINEAR_Y, "Linear GRAY"},
    {PNG_FORMAT_LINEAR_Y_ALPHA, "Linear GRAY (with alpha)"},
    {PNG_FORMAT_LINEAR_RGB, "Linear RGB"},
    {PNG_FORMAT_LINEAR_RGB_ALPHA, "Linear RGBA"},
    {PNG_FORMAT_RGB_COLORMAP, "RGB with Colormap"},
    {PNG_FORMAT_BGR_COLORMAP, "BGR with Colormap"},
    {PNG_FORMAT_RGBA_COLORMAP, "RGB with Colormap"},
    {PNG_FORMAT_ARGB_COLORMAP, "ARGB with Colormap"},
    {PNG_FORMAT_BGRA_COLORMAP, "BGRA with Colormap"},
    {PNG_FORMAT_ABGR_COLORMAP, "ABGR with Colormap"}};
auto _string_format_map = _mirror_map(_format_string_map);

const std::string& format_to_string(png_uint_32 format) {
	if (_format_string_map.contains(format))
		return _format_string_map.at(format);
	throw ssimp::exceptions::Unsupported(
	    std::format("Unrecognized format ID, ID: {}", format));
}

png_uint_32 string_to_format(const std::string& format) {
	if (_string_format_map.contains(format))
		return _string_format_map.at(format);
	throw ssimp::exceptions::Unsupported(
	    std::format("Format {} is not supported", format));
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
	if constexpr (std::is_same_v<T, img::GRAY8A>)
		image.format = PNG_FORMAT_GA;
	else if constexpr (std::is_same_v<T, img::RGB8>)
		image.format = PNG_FORMAT_RGB;
	else if constexpr (std::is_same_v<T, img::RGBA8>)
		image.format = PNG_FORMAT_RGBA;
	if (std::get<bool>(options.at("fast_save")))
		image.flags |= PNG_IMAGE_FLAG_FAST;

	const void* buffer = img_.span().data();

	std::string colorspace = std::get<std::string>(options.at("colorspace"));

	std::vector<std::byte> new_image_buffer;
	if (colorspace != "auto") {
		if (PNG_IMAGE_SAMPLE_CHANNELS(image.format) !=
		    PNG_IMAGE_SAMPLE_CHANNELS(string_to_format(colorspace)))
			throw ssimp::exceptions::Unsupported(std::format(
			    "Cannot convert images to different channel size, got: {}, "
			    "requested: {}",
			    PNG_IMAGE_SAMPLE_CHANNELS(image.format),
			    PNG_IMAGE_SAMPLE_CHANNELS(string_to_format(colorspace))));

		png_alloc_size_t mem_size;
		if (!png_image_write_get_memory_size(image, mem_size, false, buffer, 0,
		                                     nullptr))
			throw ssimp::exceptions::IOError(std::format(
			    "Obtaining memory requirements ended with message: '{}'",
			    image.message));
		std::vector<std::byte> mem_image(mem_size);
		if (!png_image_write_to_memory(&image, mem_image.data(), &mem_size,
		                               false, buffer, 0, nullptr))
			throw ssimp::exceptions::IOError(std::format(
			    "Writing to memory ended with message: '{}'", image.message));

		png_image new_image;
		std::memset(&new_image, 0, sizeof(png_image));
		new_image.version = PNG_IMAGE_VERSION;
		if (!png_image_begin_read_from_memory(&new_image, mem_image.data(),
		                                      mem_image.size()))
			throw ssimp::exceptions::IOError(std::format(
			    "Begin reading of image from memory ended with message: '{}'",
			    new_image.message));

		new_image.format = string_to_format(colorspace);
		new_image_buffer.resize(PNG_IMAGE_SIZE(new_image));
		if (!png_image_finish_read(&new_image, nullptr, new_image_buffer.data(),
		                           0, nullptr))
			throw ssimp::exceptions::IOError(std::format(
			    "Finish reading of image from memory ended with message: '{}'",
			    new_image.message));
		buffer = new_image_buffer.data();
		image.format = new_image.format;
	}

	if (!png_image_write_to_file(&image, str_path.c_str(), false, buffer, 0,
	                             nullptr))
		throw ssimp::exceptions::IOError(std::format(
		    "Could not write: {}; (message: '{}')", str_path, image.message));
}

INSTANTIATE_SAVE_TEMPLATE(PNG, img::GRAY8);
INSTANTIATE_SAVE_TEMPLATE(PNG, img::GRAY8A);
INSTANTIATE_SAVE_TEMPLATE(PNG, img::RGB8);
INSTANTIATE_SAVE_TEMPLATE(PNG, img::RGBA8);

} // namespace ssimp::formats
