/*
 *  This file is part of nzbget. See <https://nzbget.com>.
 *
 *  Copyright (C) 2025 Denis <denis@nzbget.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CHECKS_H
#define CHECKS_H

#include <functional>
#include "Options.h"
#include "Check.h"

namespace HealthCheck::Checks
{
	using CheckFunc = std::function<Check()>;

	template<typename... Checks>
	inline auto ComposeChecks(Checks... checks)
	{
		return [=](auto&&... args) -> Check
		{
			Check result = Check::Ok();
			([&] {
				result = std::invoke(checks, std::forward<decltype(args)>(args)...);
				return result.IsOk();
			}() && ...);
			return result;
		};
	}

	Check CheckRequiredOption(std::string_view name, std::string_view value);
	Check CheckValueUnique(
		std::string_view name, 
		std::string_view value, 
		const std::vector<std::pair<std::string_view, std::string_view>>& otherValues);
	Check CheckRequiredOption(std::string_view name, std::string_view value);
	Check CheckRequiredDir(std::string_view name, std::string_view value);
	Check CheckOptionalDir(std::string_view name, std::string_view value);

	Check CheckInterDirConfiguration(const Options& options);
	Check CheckLoggingConfiguration(const Options& options);
	Check CheckCertStoreConfiguration(const Options& options);

#ifndef _WIN32
	Check CheckLockFileConfiguration(const Options& options);
#endif

	namespace File
	{
		Check Exists(std::string_view name, std::string_view value);
		Check Readable(std::string_view name, std::string_view value);
		Check Writable(std::string_view name, std::string_view value);
		Check Executable(std::string_view name, std::string_view value);
	}

	namespace Directory
	{
		Check Readable(std::string_view name, std::string_view value);
		Check Writable(std::string_view name, std::string_view value);
	}
}

#endif
