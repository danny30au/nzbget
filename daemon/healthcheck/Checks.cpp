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

#include "nzbget.h"

#include "Checks.h"

namespace fs = boost::filesystem;
using namespace boost::system;

namespace HealthCheck::Checks
{
	Check CheckRequiredOption(std::string_view name, std::string_view value)
	{
		if (value.empty())
		{
			return Check::Error(std::string(name) + " is required and cannot be empty.");
		}

		return Check::Ok();
	}

	Check CheckInterDirConfiguration(const Options& options)
	{
		std::string_view value = options.GetInterDir();
		if (value.empty())
		{
			return Check::Warning(std::string(Options::INTERDIR) + 
				" is set to empty. Using InterDir is recommended for optimal unpack performance.");
		}

		return Check::Ok();
	}

	Check CheckLoggingConfiguration(const Options& options)
	{
		std::string_view logFile = options.GetLogFile();
		const auto writeLog = options.GetWriteLog();

		if (logFile.empty() && writeLog != Options::EWriteLog::wlNone)
		{
			return Check::Error(
				std::string(Options::WRITELOG) + 
				" is enabled, but " + 
				std::string(logFile) + 
				" path is empty.");
		}

		if (!logFile.empty() && writeLog == Options::EWriteLog::wlNone)
		{
			return Check::Warning(
				std::string(Options::WRITELOG) + 
				" is set to 'None', but logging is disabled.");
		}

		if (logFile.empty() && writeLog == Options::EWriteLog::wlNone)
		{
			return Check::Info("Logging is disabled. Logging is recommended for effective debugging and troubleshooting.");
		}

		return Check::Ok();
	}

	Check CheckCertStoreConfiguration(const Options& options)
	{
		std::string_view certStore = options.GetCertStore();
		bool certCheck = options.GetCertCheck();
		if (!certCheck)
			return Check::Ok();

		if (!fs::is_directory(certStore))
			return Check::Ok();

		if (certCheck && certStore.empty())
			return Check::Warning(
		std::string(Options::CERTCHECK) + " is enabled but " + 
			std::string(Options::CERTSTORE) + " is empty.");

		Check check = File::Exists(Options::CERTSTORE, options.GetCertStore());
		if (!check.IsOk())
			return check;

		return File::Readable(Options::CERTSTORE, options.GetCertStore());
	}

#ifndef _WIN32
	Check CheckLockFileConfiguration(const Options& options)
	{
		std::string_view lockFile = options.GetLockFile();
		if (lockFile.empty() && options.GetDaemonMode())
		{
			Check::Warning(
				std::string(Options::LOCKFILE) + " value is empty. The check for another running instance is disabled.");
		}

		return Check::Ok();
	}
#endif

	Check CheckValueUnique(
		std::string_view name, 
		std::string_view value, 
		const std::vector<std::pair<std::string_view, std::string_view>>& otherValues)
	{
		const auto found = std::find_if(
			otherValues.cbegin(), otherValues.cend(),
			[&](const auto& pair)
			{
				return pair.second == value;
			}
		);

		if (found == otherValues.end())
			return Check::Ok();

		return Check::Warning(std::string(name) + " and " + std::string(found->first) + " are the same that can lead to unexpected behavior");
	}

	Check CheckRequiredDir(std::string_view name, std::string_view path)
	{
		error_code ec;
		if (!fs::exists(path, ec))
		{
			return Check::Error(std::string(name) + " directory doesn't exist");
		}

		if (!fs::is_directory(path, ec))
		{
			return Check::Error(std::string(name) + " must be directory, not a file");
		}

		return Check::Ok();
	}

	Check CheckOptionalDir(std::string_view name, std::string_view path)
	{
		error_code ec;
		if (!fs::exists(path, ec))
		{
			return Check::Warning(std::string(name) + " directory doesn't exist");
		}

		if (!fs::is_directory(path, ec))
		{
			return Check::Error(std::string(name) + " is not a directory");
		}

		return Check::Ok();
	}

	namespace File
	{
		Check Exists(std::string_view name, std::string_view value)
		{
			error_code ec;

			if (!fs::exists(value, ec))
			{
				return Check::Error(std::string(name) + " file doesn't exist");
			}

			if (!fs::is_regular_file(value))
			{
				return Check::Error(std::string(name) + " must be a file");
			}

			return Check::Ok();

		}

		Check Readable(std::string_view name, std::string_view value)
		{
			const fs::path path(value);
			std::ifstream file(path.c_str());
			if (file.is_open())
			{
				return Check::Ok();
			}
			return Check::Error(std::string(name) + " file is not readable: " + std::strerror(errno));
		}

		Check Writable(std::string_view name, std::string_view value)
		{
			const fs::path path(value);
			std::ofstream file(path.c_str(), std::ios::app);
			if (file.is_open())
			{
				return Check::Ok();
			}
			return Check::Error(std::string(name) + " file is not writable: " + std::strerror(errno));
		}

		Check Executable(std::string_view name, std::string_view value)
		{
#ifdef _WIN32
			const fs::path path(value);
			if (!path.has_extension())
			{
				return Check::Error(std::string(name) + " file is not executable: no extension");
			}
			const auto ext = path.extension();
			if (ext == ".exe" || ext == ".bat" || ext == ".cmd" || ext == ".com")
			{
				return Check::Ok();
			}

			return Check::Error(std::string(name) + " file is not executable: extension is " + ext.string());
#else
			if (access(value.data(), X_OK) == 0)
			{
				return Check::Ok();
			}
			return Check::Error(std::string(name) + " file is not executable: " + std::strerror(errno));
#endif
		}
	}

	namespace Directory
	{
		Check Readable(std::string_view name, std::string_view path)
		{
			error_code ec;
			fs::directory_iterator(path, ec);
			if (ec)
			{
				return Check::Error(std::string(name) + " directory must be readable");
			}

			return Check::Ok();
		}

		Check Writable(std::string_view name, std::string_view path)
		{
			error_code ec;
			const fs::path dirPath(path);
			const fs::path testPath = dirPath / "nzbget_write_test.txt";

			{
				std::ofstream testFile(testPath.c_str());
				if (!testFile.is_open())
				{
					return Check::Error(
						std::string("Failed to create test file in ") + 
						std::string(name) +
						": " + std::strerror(errno)
					);
				}

				testFile << "Write test";
				if (testFile.fail())
				{
					return Check::Error(
						std::string("Failed to write to test file in ") + 
						std::string(name) + 
						": " + std::strerror(errno)
					);
				}
			}

			fs::remove(testPath, ec);
			if (ec)
			{
				return Check::Error(
					"Failed to remove test file from " + std::string(name) + 
					": " + ec.message());
			}

			return Check::Ok();
		}
	}
}
