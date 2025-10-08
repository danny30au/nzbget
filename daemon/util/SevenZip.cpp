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
#include <array>
#include <regex>
#include "Unpack.h"
#include "Util.h"

using namespace Unpack;

/**
 * <Commands>
 *		x : eXtract files with full paths
 * <Switches>
 *		-y : assume Yes on all queries
 *		-ao{a|s|t|u} : set Overwrite mode
 *		-p{Password} : set Password
 *		-o{Directory} : set Output directory
*/
std::string SevenZip::BuildCommand() const
{
	auto command = "\"" + m_tool.string() + "\" x -y";

	switch (m_mode)
	{
	case OverwriteMode::Skip: command += " -aos"; break;
	case OverwriteMode::Overwrite: command += " -aoa"; break;
	case OverwriteMode::AutoRename: command += " -aou"; break;
	}

	command += BuildPassword();

	command += "-o\"" + m_outputDir.string() + "\"";
	command += " \"" + m_archive.string() + "\"";
	command += Util::NULL_OUTPUT;

#ifdef _WIN32
	command = "\"" + command + "\"";
#endif

	return command;
}

bool SevenZip::IsSupported(const boost::filesystem::path& path)
{
	if (!path.has_filename() || !path.has_extension())
		return false;

	std::string filename = path.filename().string();
	std::transform(filename.begin(), filename.end(), filename.begin(),
		[](unsigned char c) { return std::tolower(c); });
	const static std::array<std::string_view, 9> formats{
		".7z", ".zip", ".7z.001", ".tar", ".gz", ".bz", ".bz2", ".tgz", ".txz"
	};

	return std::any_of(formats.cbegin(), formats.cend(),
		[&](std::string_view ext)
		{
			return Util::EndsWith(filename, ext);
		});
}

std::string_view SevenZip::DecodeExitCode(int ec) const
{
	switch (static_cast<ExitCode>(ec))
	{
	case ExitCode::Success:
		return "Operation completed successfully.";

	case ExitCode::Warning:
		return "Completed with warnings. Some files may have been skipped because they were in use.";

	case ExitCode::FatalError:
		return "A fatal error occurred. Check for permission issues, a corrupt archive, password, disk space.";

	case ExitCode::CmdLineError:
		return "Command line error.";

	case ExitCode::NotEnoughMemoryError:
		return "Not enough memory for operation.";

	case ExitCode::CanceledByUser:
		return "User stopped the process.";

	default:
		return "Unknown 7-Zip error.";
	};
}
