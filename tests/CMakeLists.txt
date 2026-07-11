cmake_minimum_required(VERSION 3.16)

find_package(Qt6 COMPONENTS Core Test Sql Network Concurrent Qml QuickTest REQUIRED)

set(CMAKE_AUTOMOC ON)

# Database Manager Test
add_executable(tst_database_manager tst_database_manager.cpp ../nomad-llm/src/database_manager.cpp)
target_include_directories(tst_database_manager PRIVATE ../nomad-llm/src)
target_link_libraries(tst_database_manager PRIVATE Qt6::Core Qt6::Test Qt6::Sql)
add_test(NAME tst_database_manager COMMAND tst_database_manager -xml -o test_database_manager_result.xml)

# Settings Manager Test
add_executable(tst_settings_manager tst_settings_manager.cpp ../nomad-llm/src/settings_manager.cpp)
target_include_directories(tst_settings_manager PRIVATE ../nomad-llm/src)
target_link_libraries(tst_settings_manager PRIVATE Qt6::Core Qt6::Test)
add_test(NAME tst_settings_manager COMMAND tst_settings_manager -xml -o test_settings_manager_result.xml)

# Hardware Detector Test
add_executable(tst_hardware_detector tst_hardware_detector.cpp ../nomad-llm/src/hardware_detector.cpp)
target_include_directories(tst_hardware_detector PRIVATE ../nomad-llm/src)
target_link_libraries(tst_hardware_detector PRIVATE Qt6::Core Qt6::Test)
add_test(NAME tst_hardware_detector COMMAND tst_hardware_detector -xml -o test_hardware_detector_result.xml)

# Memory Manager Test (Needs DatabaseManager and InferenceEngine)
add_executable(tst_memory_manager tst_memory_manager.cpp ../nomad-llm/src/memory_manager.cpp ../nomad-llm/src/database_manager.cpp ../nomad-llm/src/inference_engine.cpp ../nomad-llm/src/LlamaBackend.cpp)
target_include_directories(tst_memory_manager PRIVATE ../nomad-llm/src ${llama_cpp_SOURCE_DIR}/include ${llama_cpp_SOURCE_DIR}/common ${llama_cpp_SOURCE_DIR}/ggml/include)
target_link_libraries(tst_memory_manager PRIVATE Qt6::Core Qt6::Test Qt6::Sql Qt6::Concurrent llama common)
add_test(NAME tst_memory_manager COMMAND tst_memory_manager -xml -o test_memory_manager_result.xml)

# Document Processor Test
add_executable(tst_document_processor tst_document_processor.cpp ../nomad-llm/src/document_processor.cpp ../nomad-llm/src/database_manager.cpp)
target_include_directories(tst_document_processor PRIVATE ../nomad-llm/src)
target_link_libraries(tst_document_processor PRIVATE Qt6::Core Qt6::Test Qt6::Sql)
add_test(NAME tst_document_processor COMMAND tst_document_processor -xml -o test_doc_proc_result.xml)

# Model Manager Test
add_executable(tst_model_manager tst_model_manager.cpp ../nomad-llm/src/model_manager.cpp ../nomad-llm/src/settings_manager.cpp)
target_include_directories(tst_model_manager PRIVATE ../nomad-llm/src)
target_link_libraries(tst_model_manager PRIVATE Qt6::Core Qt6::Network Qt6::Test)
add_test(NAME tst_model_manager COMMAND tst_model_manager -xml -o test_model_manager_result.xml)

# Inference Engine Test
add_executable(tst_inference_engine tst_inference_engine.cpp ../nomad-llm/src/inference_engine.cpp ../nomad-llm/src/LlamaBackend.cpp)
target_include_directories(tst_inference_engine PRIVATE ../nomad-llm/src ${llama_cpp_SOURCE_DIR}/include ${llama_cpp_SOURCE_DIR}/common ${llama_cpp_SOURCE_DIR}/ggml/include)
target_link_libraries(tst_inference_engine PRIVATE Qt6::Core Qt6::Test Qt6::Concurrent llama common)
add_test(NAME tst_inference_engine COMMAND tst_inference_engine -xml -o test_inference_engine_result.xml)

# QML UI Tests (Hybrid Runner)
add_executable(tst_ui_runner tst_ui_runner.cpp)
target_include_directories(tst_ui_runner PRIVATE ../nomad-llm/src)
target_link_libraries(tst_ui_runner PRIVATE Qt6::Core Qt6::Test Qt6::Qml Qt6::QuickTest)
target_compile_definitions(tst_ui_runner PRIVATE QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/qml")
# We need to compile the QML file into the test or load it dynamically
add_test(NAME tst_ui_runner COMMAND tst_ui_runner -xml -o test_ui_runner_result.xml)
