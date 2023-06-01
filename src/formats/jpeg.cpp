#include "jpeg.hpp"
#include "common.hpp"
#include <turbojpeg.h>

namespace ssimp::formats {
/* static */ std::vector<img::LocalizedImage>
JPEG::load_image(const fs::path& path) {
	tjhandle decompressor = tjInitDecompress();

	return {};
}

/* static */
std::optional<ImageProperties> JPEG::get_information(const fs::path& path) {
	tjhandle decompressor = tjInitDecompress();

	int width, height, jpegSubsamp, jpegColorspace;
	auto bytes = details::read_file(path);

	tjDecompressHeader3(
	    decompressor, reinterpret_cast<const unsigned char*>(bytes.data()),
	    bytes.size(), &width, &height, &jpegSubsamp, &jpegColorspace);

	return ImageProperties(name, {std::size_t(width), std::size_t(height)}, {});
}

template <typename T>
    requires mt::traits::is_any_of_tuple_v<T, JPEG::supported_types>
/* static */ void JPEG::save_image(const img::ndImage<T>& img,
                                   const fs::path& path,
                                   const option_types::options_t& options) {}

INSTANTIATE_SAVE_TEMPLATE(JPEG, img::GRAY8);
} // namespace ssimp::formats
