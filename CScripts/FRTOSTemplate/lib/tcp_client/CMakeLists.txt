add_library(tcp_client_lib 
src/picow_tcp_client.c)

target_compile_definitions(tcp_client_lib PRIVATE
WIFI_SSID=\"${WIFI_SSID}\"
WIFI_PASSWORD=\"${WIFI_PASSWORD}\")
		
target_include_directories(tcp_client_lib PUBLIC include)

target_link_libraries(tcp_client_lib
pico_stdlib
pico_cyw43_arch_lwip_threadsafe_background)
		


