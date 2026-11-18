target_sources(SR PRIVATE
	CoD4.hpp
	CoD4.cpp
	CS.hpp
	CS.cpp
	Monkey.hpp
	Monkey.cpp
	Q3.hpp
	Q3.cpp
	Q3Weapons.hpp
	Q3Weapons.cpp
	Racer.hpp
	Racer.cpp)

# The same arithmetic as the clients that predict these: no x87, no contraction. Source properties only reach
# targets made in the folder they are set in, and SR is made in src: without TARGET_DIRECTORY these were x87.
set_source_files_properties(Monkey.cpp Racer.cpp TARGET_DIRECTORY SR PROPERTIES
	COMPILE_OPTIONS "-msse2;-mfpmath=sse;-ffp-contract=off"
	SKIP_PRECOMPILE_HEADERS ON)
