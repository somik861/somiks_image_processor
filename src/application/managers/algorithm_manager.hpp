#pragma once

#include "../../algorithms/blur.hpp"
#include "../../algorithms/change_type.hpp"
#include "../../algorithms/fft.hpp"
#include "../../algorithms/split_channels.hpp"
#include "../../algorithms/unary_math.hpp"
#include "../../algorithms/resize.hpp"
#include "../nd_image.hpp"
#include "_algo_format_base.hpp"
#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace ssimp {
class AlgorithmManager : public details::_AlgoFormatBase {
  private:
	using _registered_algorithms = std::tuple<algorithms::SplitChannels,
	                                          algorithms::ChangeType,
	                                          algorithms::Blur,
	                                          algorithms::FFT,
	                                          algorithms::UnaryMath,
						  algorithms::Resize>;

  public:
	/**
	 * Modified constructor for algorithm manager initialization.
	 */
	AlgorithmManager();

	/**
	 * Apply **algorithm** to images.
	 */
	std::vector<img::LocalizedImage>
	apply(const std::vector<img::ndImageBase>& images,
	      const std::string& algorithm,
	      const option_types::options_t& options) const;

  private:
	using _applier_t = std::function<std::vector<img::LocalizedImage>(
	    const std::vector<img::ndImageBase>&, const option_types::options_t&)>;

	_funmap_t<_applier_t> _appliers;
};
} // namespace ssimp
