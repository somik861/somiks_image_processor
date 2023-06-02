#include "jpeg.hpp"
#include "common.hpp"
#include <optional>
#include <ostream>
#include <turbojpeg.h>

namespace {
class JpegMeta {
  public:
	std::size_t width;
	std::size_t height;
	TJSAMP jpeg_subsampling;
	int jpeg_colorspace;
};

std::optional<JpegMeta> jpeg_info(tjhandle decompressor,
                                  std::span<const std::byte> bytes) {
	int _width, _height, _jpeg_subsampling, _jpeg_colorspace;
	JpegMeta out;

	if (tjDecompressHeader3(
	        decompressor, reinterpret_cast<const unsigned char*>(bytes.data()),
	        bytes.size(), &_width, &_height, &_jpeg_subsampling,
	        &_jpeg_colorspace))
		return {};

	out.height = std::size_t(_height);
	out.width = std::size_t(_width);
	out.jpeg_subsampling = static_cast<TJSAMP>(_jpeg_subsampling);
	out.jpeg_colorspace = _jpeg_colorspace;
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

} // namespace

namespace ssimp::formats {
/* static */ std::vector<img::LocalizedImage>
JPEG::load_image(const fs::path& path) {
	tjhandle decompressor = tjInitDecompress();

	return {};
}

/* static */
std::optional<ImageProperties> JPEG::get_information(const fs::path& path) {
	tjhandle decompressor = tjInitDecompress();
	auto bytes = details::read_file(path);

	std::optional<JpegMeta> meta_data = jpeg_info(decompressor, bytes);
	if (!meta_data)
		return {};

	ImageProperties out(name, {meta_data->width, meta_data->height}, {});
	out.others["Chroma Subsampling"] = "";
	out.others["Colorspace"] = "";

	return out;
}

template <typename T>
    requires mt::traits::is_any_of_tuple_v<T, JPEG::supported_types>
/* static */ void JPEG::save_image(const img::ndImage<T>& img,
                                   const fs::path& path,
                                   const option_types::options_t& options) {}

INSTANTIATE_SAVE_TEMPLATE(JPEG, img::GRAY8);
} // namespace ssimp::formats
