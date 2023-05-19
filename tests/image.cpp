#include "../src/application/image.hpp"
#include "common.hpp"

using type_list = img::type_list;

TEMPLATE_LIST_TEST_CASE("ndImage", "ndImage[template]", type_list) {}
