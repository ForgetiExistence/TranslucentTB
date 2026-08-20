#pragma once
#include "arch.h"
#include <spdlog/common.h>
#include <source_location>
#include <string_view>
#include <thread>

#include "../api.h"
#include "appinfo.hpp"
#include "util/null_terminated_string_view.hpp"

#define UTF8_ERROR_TITLE UTF8_APP_NAME " " UTF8_APP_VERSION " - Error"

namespace Error {
	namespace impl {
		PROGRAMLOG_API bool ShouldLogInternal(spdlog::level::level_enum level);

		// Needs to be in DLL because spdlog log registry is per-module.
		PROGRAMLOG_API void Log(std::wstring_view msg, spdlog::level::level_enum level, std::source_location location);

		PROGRAMLOG_API std::wstring GetLogMessage(std::wstring_view message, std::wstring_view error_message);

		PROGRAMLOG_API void HandleError(std::wstring_view message, std::wstring_view error_message, std::source_location location);
		[[noreturn]] PROGRAMLOG_API void HandleCritical(std::wstring_view message, std::wstring_view error_message, std::source_location location);

		template<spdlog::level::level_enum level>
		inline void Handle(std::wstring_view message, std::wstring_view error_message, std::source_location location)
		{
			if constexpr (level == spdlog::level::err)
			{
				HandleError(message, error_message, location);
			}
			else if constexpr (level == spdlog::level::critical)
			{
				HandleCritical(message, error_message, location);
			}
			else
			{
				Log(GetLogMessage(message, error_message), level, location);
			}
		}

		std::thread HandleCommon(spdlog::level::level_enum level, std::wstring_view message, std::wstring_view error_message, std::source_location location, Util::null_terminated_wstring_view title, std::wstring_view description, unsigned int type);
		void HandleCriticalCommon(std::wstring_view message, std::wstring_view error_message, std::source_location location);
	}

	template<spdlog::level::level_enum level>
	inline bool ShouldLog()
	{
		if constexpr (level == spdlog::level::critical || level == spdlog::level::err)
		{
			return true;
		}
		else
		{
			return impl::ShouldLogInternal(level);
		}
	}
};

#define PROGRAMLOG_ERROR_LOCATION std::source_location::current()
#define MessagePrint(level_, message_) Error::impl::Handle<(level_)>((message_), std::wstring_view { }, PROGRAMLOG_ERROR_LOCATION)
#define ErrorHandleCommonMacro(level_, message_, error_message_) do { \
	if constexpr ((level_) == spdlog::level::critical || (level_) == spdlog::level::err) \
	{ \
		Error::impl::Handle<(level_)>((message_), (error_message_), PROGRAMLOG_ERROR_LOCATION); \
	} \
	else if (Error::ShouldLog<(level_)>()) \
	{ \
		Error::impl::Handle<(level_)>((message_), (error_message_), PROGRAMLOG_ERROR_LOCATION); \
	} \
} while (0)
