add_subdirectory(Demo)
add_subdirectory(Entity)
add_subdirectory(Player)

target_sources(SR PRIVATE
	Map.hpp
	Map.cpp
	Server.hpp
	Server.cpp
	VR.hpp
	VR.cpp)
