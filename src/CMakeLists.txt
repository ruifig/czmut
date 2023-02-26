add_executable(examples
	"mymain.cpp"

	"../lib/examples/common/common.h"
	"../lib/examples/example_basic.h"
	"../lib/examples/example_sections.h"
	"../lib/examples/example_templated.h"
)

target_link_libraries(examples czmut)

cz_setCommonBinaryProperties(examples "/")
