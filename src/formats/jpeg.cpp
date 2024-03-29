#include "jpeg.hpp"
#include "common_macro.hpp"

#include <turbojpeg.h>

namespace {
class JpegMeta {
  public:
	int width;
	int height;
	TJSAMP jpeg_subsampling;
	TJCS jpeg_colorspace;
};

std::optional<JpegMeta> jpeg_info(tjhandle decompressor,
                                  std::span<const std::byte> bytes) {
	int _jpeg_subsampling, _jpeg_colorspace;
	JpegMeta out;

	if (tjDecompressHeader3(
	        decompressor, reinterpret_cast<const unsigned char*>(bytes.data()),
	        bytes.size(), &out.width, &out.height, &_jpeg_subsampling,
	        &_jpeg_colorspace))
		return {};

	out.jpeg_subsampling = static_cast<TJSAMP>(_jpeg_subsampling);
	out.jpeg_colorspace = static_cast<TJCS>(_jpeg_colorspace);
	return out;
}

std::ostream& operator<<(std::ostream& os, TJSAMP samp) {
	switch (samp) {
	case TJSAMP_444:
		return os << "4:4:4";
	case TJSAMP_422:
		return os << "4:2:2";
	case TJSAMP_420:
		return os << "4:2:0";
	case TJSAMP_GRAY:
		return os << "GRAY";
	case TJSAMP_440:
		return os << "4:4:0";
	case TJSAMP_411:
		return os << "4:1:1";
	default:
		return os << "unkown";
	}
}

std::ostream& operator<<(std::ostream& os, TJCS samp) {
	switch (samp) {
	case TJCS_RGB:
		return os << ssimp::img::elem_type::RGB_8;
	case TJCS_YCbCr:
		return os << "YCbCr";
	case TJCS_GRAY:
		return os << ssimp::img::elem_type::GRAY_8;
	case TJCS_CMYK:
		return os << "CMYK";
	case TJCS_YCCK:
		return os << "YCCK";
	default:
		return os << "unkown";
	}
}

TJSAMP samp_from_string(std::string_view sv) {
	if (sv == "4:4:4")
		return TJSAMP_444;
	if (sv == "4:2:2")
		return TJSAMP_422;
	if (sv == "4:2:0")
		return TJSAMP_420;
	if (sv == "GRAY")
		return TJSAMP_GRAY;
	if (sv == "4:4:0")
		return TJSAMP_440;
	if (sv == "4:1:1")
		return TJSAMP_411;

	throw std::runtime_error("Invalid value");
}

template <typename T>
std::pair<ssimp::img::ndImageBase, unsigned char*> get_image(int width,
                                                             int height) {
	ssimp::img::ndImage<T> img(width, height);

	return {img, reinterpret_cast<unsigned char*>(img.span().data())};
}
} // namespace

namespace ssimp::formats {
/* static */
bool JPEG::image_count_supported(std::size_t count) { return count == 1; }

/* static */
bool JPEG::image_dims_supported(std::span<const std::size_t> dims) {
	return dims.size() == 2 && dims[0] > 0 && dims[1] > 0;
}

/* static */ std::optional<std::vector<img::LocalizedImage>>
JPEG::load_image(const fs::path& path, const option_types::options_t&) {
	tjhandle decompressor = tjInitDecompress();
	auto bytes = details::read_file(path);

	auto meta_data = jpeg_info(decompressor, bytes);
	if (!meta_data) {
		tjDestroy(decompressor);
		return {};
	}

	bool gray = meta_data->jpeg_colorspace == TJCS_GRAY;

	TJPF pixel_format = gray ? TJPF_GRAY : TJPF_RGB;

	img::ndImageBase dest_img = img::ndImage<img::GRAY_8>(1);
	unsigned char* dest_ptr;
	if (gray)
		std::tie(dest_img, dest_ptr) =
		    get_image<img::GRAY_8>(meta_data->width, meta_data->height);
	else
		std::tie(dest_img, dest_ptr) =
		    get_image<img::RGB_8>(meta_data->width, meta_data->height);

	int rv = tjDecompress2(decompressor,
	                       reinterpret_cast<const unsigned char*>(bytes.data()),
	                       bytes.size(), dest_ptr, meta_data->width, 0,
	                       meta_data->height, pixel_format, 0);
	tjDestroy(decompressor);
	if (rv)
		return {};

	return std::vector<img::LocalizedImage>{{dest_img}};
}

/* static */
std::optional<ImageProperties>
JPEG::get_information(const fs::path& path, const option_types::options_t&) {
	tjhandle decompressor = tjInitDecompress();
	auto bytes = details::read_file(path);

	std::optional<JpegMeta> meta_data = jpeg_info(decompressor, bytes);
	tjDestroy(decompressor);
	if (!meta_data)
		return {};

	ImageProperties out(
	    name, {std::size_t(meta_data->width), std::size_t(meta_data->height)},
	    {});
	out.others["Chroma Subsampling"] = to_string(meta_data->jpeg_subsampling);
	out.others["Colorspace"] = to_string(meta_data->jpeg_colorspace);

	return out;
}

template <typename T>
    requires mt::traits::is_any_of_tuple_v<T, JPEG::supported_types>
/* static */ void JPEG::save_image(const std::vector<img::ndImage<T>>& imgs,
                                   const fs::path& path,
                                   const option_types::options_t& options) {
	auto& img = imgs[0];
	tjhandle compressor = tjInitCompress();

	unsigned char* jpeg_buffer = nullptr;
	unsigned long jpeg_size;

	bool gray = std::is_same_v<T, img::GRAY_8>;

	if (tjCompress2(
	        compressor,
	        reinterpret_cast<const unsigned char*>(img.span().data()),
	        img.dims()[0], 0, img.dims()[1], gray ? TJPF_GRAY : TJPF_RGB,
	        &jpeg_buffer, &jpeg_size,
	        static_cast<int>(gray ? TJSAMP_GRAY
	                              : samp_from_string(std::get<std::string>(
	                                    options.at("subsampling")))),
	        std::get<int32_t>(options.at("quality")), 0)) {
		std::cerr << "Jpeg compressor error\n";
	}

	tjDestroy(compressor);
	details::save_file(
	    path, std::span<const std::byte>(
	              reinterpret_cast<const std::byte*>(jpeg_buffer), jpeg_size));

	tjFree(jpeg_buffer);
}

INSTANTIATE_SAVE_TEMPLATE(JPEG, img::GRAY_8);
INSTANTIATE_SAVE_TEMPLATE(JPEG, img::RGB_8);
} // namespace ssimp::formats
