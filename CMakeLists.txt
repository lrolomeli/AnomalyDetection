if (PICO_CYW43_SUPPORTED) # set by PICO_BOARD=pico_w
    if (NOT TARGET pico_cyw43_arch)
        message("Skipping Pico W examples as support is not available")
    else()

        if (DEFINED ENV{WIFI_SSID} AND (NOT WIFI_SSID))
            set(WIFI_SSID $ENV{WIFI_SSID})
            message("Using WIFI_SSID from environment ('${WIFI_SSID}')")
        endif()

        if (DEFINED ENV{WIFI_PASSWORD} AND (NOT WIFI_PASSWORD))
            set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})
            message("Using WIFI_PASSWORD from environment")
        endif()

        set(WIFI_SSID "${WIFI_SSID}" CACHE INTERNAL "WiFi SSID for examples")
        set(WIFI_PASSWORD "${WIFI_PASSWORD}" CACHE INTERNAL "WiFi password for examples")

        add_subdirectory(wifi)
    endif()
endif()


if ("${WIFI_SSID}" STREQUAL "")
	message("Skipping some Pico W examples as WIFI_SSID is not defined")
elseif ("${WIFI_PASSWORD}" STREQUAL "")
	message("Skipping some Pico W examples as WIFI_PASSWORD is not defined")
else()
	add_subdirectory_exclude_platforms(tcp_client)
endif()
