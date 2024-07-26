idf_component_register(SRCS "semaphore_tasks.c" "main.c" "simple_tasks.c"
                    INCLUDE_DIRS ".")

target_compile_definitions(${COMPONENT_LIB} PRIVATE
    configIDLE_SHOULD_YIELD=1
)