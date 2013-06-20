foreach(size 16 22 24 32 48 64 128 256)
	install(DIRECTORY ${size}x${size} DESTINATION ${prefix}/share/icons/hicolor
		PATTERN .* EXCLUDE
		PATTERN *~ EXCLUDE)
	list(APPEND icons ${size}x${size})
endforeach(size 16 22 24 32 48 64 128 256)

list(APPEND icons "scalable")
install(DIRECTORY scalable DESTINATION ${prefix}/share/icons/hicolor)
install(SCRIPT ${XMR_CMAKE_PATH}/update-icon-cache.cmake)

generate_dirs("${icons}" "icons/hicolor" "icons_data")
