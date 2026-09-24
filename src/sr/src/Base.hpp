#pragma once
#include <algorithm>
#include <array>
#include <atomic>
#include <charconv>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

#include "Macros.hpp"

IZ_C
{
	// clang-format off
	#include <cscr_variable.h>
	#include <cscr_stringlist.h>
	#include <g_shared.h>
	#include <huffman.h>
	#include <net_game.h>
	#include <qcommon_logprint.h>
	#include <scr_vm.h>
	#include <server.h>
	#include <sv_snapshot.h>
	#include <sys_main.h>
	#include <xassets.h>
	#include <bg_public.h>
	// clang-format on
}
// clang-format off
namespace SR { };
using namespace SR;
// clang-format on
