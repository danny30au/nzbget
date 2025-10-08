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

#include <sstream>
#include <regex>
#include "Unpack.h"
#include "Util.h"

using namespace Unpack;

/**
 * <Commands>
 * 		x : Extract files with full path
 * <Switches>
 *		-y : Assume Yes on all queries
 *		-ai : Ignore file attributes
 *		-o{+|-|r} : Set the overwrite mode
 *		-p{Password} : set Password
*/
std::string Unrar::BuildCommand() const
{
	auto command = "\"" + m_tool.string() + "\" x -y -ai";

	switch (m_mode)
	{
	case OverwriteMode::Skip: command += " -o-"; break;
	case OverwriteMode::Overwrite: command += " -o+"; break;
	case OverwriteMode::AutoRename: command += " -or"; break;
	}

	command += BuildPassword();
	command += "\"" + m_archive.string() + "\"";
	command += " \"" + m_outputDir.string() + "\"";
	command += Util::NULL_OUTPUT;

#ifdef _WIN32
	command = "\"" + command + "\"";
#endif

	return command;
}

bool Unrar::IsSupported(const boost::filesystem::path& path)
{
	if (!path.has_filename() || !path.has_extension())
		return false;

	std::string filename = path.filename().string();
	std::transform(filename.begin(), filename.end(), filename.begin(),
		[](unsigned char c) { return std::tolower(c); });

	static const std::regex part1Rar("\\.part0*1\\.rar$");
	if (std::regex_search(filename, part1Rar))
		return true;

	if (filename.find(".part") == std::string::npos && Util::EndsWith(filename, ".rar"))
		return true;

	return false;
}

std::string_view Unrar::DecodeExitCode(int ec) const
{
	switch (static_cast<ExitCode>(ec))
	{
	case ExitCode::Success:
		return "The archive was unpacked successfully.";

	case ExitCode::NonFatalError:
		return "Extraction finished, but some files might be missing or incomplete.";

	case ExitCode::FatalError:
		return "The process could not start or was interrupted unexpectedly.";

	case ExitCode::InvalidChecksum:
		return "The archive is damaged. The extracted files are likely corrupt.";

	case ExitCode::LockedArchive:
		return "This archive is locked and cannot be modified or unpacked.";

	case ExitCode::WriteError:
		return "Could not write files to the destination. Please check your permissions and disk space.";

	case ExitCode::FileOpenError:
		return "Couldn't open the archive. The file may be missing or you don't have permission to read it.";

	case ExitCode::CommandLineError:
		return "An internal program error occurred.";

	case ExitCode::NotEnoughMemory:
		return "Your computer ran out of memory. Please try closing other applications first.";

	case ExitCode::FileCreateError:
		return "Couldn't create files in the destination folder. Please check your permissions.";

	case ExitCode::NoFilesFound:
		return "There were no files inside the archive to extract.";

	case ExitCode::WrongPassword:
		return "The password you entered was incorrect.";

	default:
		return "Unknown Unrar error.";
	}
}
