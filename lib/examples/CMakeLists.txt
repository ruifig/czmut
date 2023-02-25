add_executable(examples
	"common/common.h"

	"example_basic.cpp"
	"example_sections.cpp"
	"example_templated.cpp"
	"main_desktop.cpp"
)

target_link_libraries(examples czmut)

cz_setCommonBinaryProperties(examples "/")

