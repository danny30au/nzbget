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

#include "Unpack.h"
#include "Options.h"

namespace Unpack
{
	Extractor::~Extractor() = default;

	ExtractorBase::ExtractorBase(
		boost::filesystem::path tool,
		boost::filesystem::path archive,
		boost::filesystem::path outputDir,
		std::string password,
		OverwriteMode mode,
		Executor executor
	)
		: m_tool(std::move(tool))
		, m_archive(std::move(archive))
		, m_outputDir(std::move(outputDir))
		, m_password(std::move(password))
		, m_mode(mode)
		, m_executor(executor)
	{
	}

	int ExtractorBase::ExecuteCommand(const char* cmd) const
	{
		int status = m_executor(cmd);
		int exitCode = -1;

#ifdef _WIN32
		exitCode = status;
#else
		if (WIFEXITED(status)) {
			exitCode = WEXITSTATUS(status);
		}
#endif
		return exitCode;
	}

	Result ExtractorBase::Extract()
	{
		const auto testCmd = BuildTestCommand();
		int exitCode = ExecuteCommand(testCmd.c_str());
		if (exitCode != 0)
			return { false, "Archive test failed: file may be corrupt or password is wrong." };

		const auto result = CheckPrerequisites();
		if (!result.success)
			return result;

		const auto cmd = BuildCommand();
		exitCode = ExecuteCommand(cmd.c_str());

		if (exitCode == 0)
			return { true, "" };

		return { false, DecodeExitCode(exitCode) };
	}

	Result ExtractorBase::CheckPrerequisites() const
	{
		boost::system::error_code ec;
		boost::filesystem::create_directories(m_outputDir, ec);
		if (ec)
			return { false, "Failed to create the output directory: " + ec.message() };

		return { true, "" };
	}

	std::string ExtractorBase::BuildPassword() const
	{
		if (m_password.empty())
			return " -p- ";
		else
			return " -p\"" + m_password + "\" ";
	}

	std::string ExtractorBase::BuildTestCommand() const
	{
		std::string command = "\"" + m_tool.string() + "\""
			+ " t"
			+ BuildPassword()
			+ "\"" + m_archive.string() + "\""
			+ Util::NULL_OUTPUT;

#ifdef _WIN32
		command = "\"" + command + "\"";
#endif

		return command;
	}

	bool IsArchive(const boost::filesystem::path& file)
	{
		return SevenZip::IsSupported(file) || Unrar::IsSupported(file);
	}

	static std::optional<boost::filesystem::path> GetToolPath(std::string_view tool)
	{
		if (boost::filesystem::is_regular_file(tool))
			return tool;

		const auto cmdLine = Util::SplitCommandLine(tool.data());
		if (cmdLine.empty())
			return std::nullopt;

		return cmdLine.front().Str();
	}

	ExtractorPtr MakeExtractor(
		boost::filesystem::path archive,
		boost::filesystem::path outputDir,
		std::string password,
		OverwriteMode mode
	)
	{
		if (SevenZip::IsSupported(archive) && g_Options->GetSevenZipCmd())
		{
			auto tool = GetToolPath(g_Options->GetSevenZipCmd());
			if (!tool)
				return nullptr;

			return std::make_unique<SevenZip>(
				std::move(*tool),
				std::move(archive),
				std::move(outputDir),
				std::move(password),
				mode,
				DefaultExecutor
			);
		}

		if (Unrar::IsSupported(archive) && g_Options->GetUnrarCmd())
		{
			auto tool = GetToolPath(g_Options->GetUnrarCmd());
			if (!tool)
				return nullptr;

			return std::make_unique<Unrar>(
				std::move(*tool),
				std::move(archive),
				std::move(outputDir),
				std::move(password),
				mode,
				DefaultExecutor
			);
		}

		return nullptr;
	}
}
