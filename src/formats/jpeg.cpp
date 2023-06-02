#include "jpeg.hpp"
#include "common.hpp"
#include <optional>
#include <turbojpeg.h>

namespace {
class JpegMeta {
  public:
	std::size_t width;
	std::size_t height;
	int jpegSubsamp;
	int jpegColorspace;
};

std::optional<JpegMeta> jpeg_info(tjhandle decompressor,
                                  std::span<const std::byte> bytes) {
	int _width, _height;
	JpegMeta out;

	if (tjDecompressHeader3(
	        decompressor, reinterpret_cast<const unsigned char*>(bytes.data()),
	        bytes.size(), &_width, &_height, &out.jpegSubsamp,
	        &out.jpegColorspace))
		return {};

	out.height = std::size_t(_height);
	out.width = std::size_t(_width);
	return out;
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
