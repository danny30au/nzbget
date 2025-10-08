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

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include "Util.h"
#include "Unpack.h"

namespace fs = boost::filesystem;
using namespace Unpack;

struct ExtractorTestFixture
{
	std::string command;
	int exitCode = 0;

	Executor executor;

	fs::path tool = "/bin/extractor";
	fs::path archive = "data.zip";
	fs::path outputDir = "out";

	ExtractorTestFixture()
	{
		executor = [&](std::string_view cmd) { command = cmd; return exitCode; };
	}

	~ExtractorTestFixture()
	{
		fs::remove_all(outputDir);
	}
};

BOOST_FIXTURE_TEST_SUITE(ExtractorLogicTests, ExtractorTestFixture)

BOOST_AUTO_TEST_CASE(SevenZipBuildsCorrectCommand)
{
	{
		std::string password = "secret";
#ifdef _WIN32
		std::string expectedCommand = "\"\"/bin/extractor\" x -y -aoa -p\"secret\" -o\"" + outputDir.string() + "\" \"" + archive.string() + "\"" + Util::NULL_OUTPUT + "\"";
#else
		std::string expectedCommand = "\"/bin/extractor\" x -y -aoa -p\"secret\" -o\"" + outputDir.string() + "\" \"" + archive.string() + "\"" + Util::NULL_OUTPUT;
#endif

		SevenZip extractor(tool, archive, outputDir, password, OverwriteMode::Overwrite, executor);
		Result res = extractor.Extract();

		BOOST_CHECK_EQUAL(command, expectedCommand);
		BOOST_CHECK(res.success);
	}

	{
		std::string password = "";
#ifdef _WIN32
		std::string expectedCommand = "\"\"/bin/extractor\" x -y -aoa -p- -o\"" + outputDir.string() + "\" \"" + archive.string() + "\"" + Util::NULL_OUTPUT + "\"";
#else
		std::string expectedCommand = "\"/bin/extractor\" x -y -aoa -p- -o\"" + outputDir.string() + "\" \"" + archive.string() + "\"" + Util::NULL_OUTPUT;
#endif
		SevenZip extractor(tool, archive, outputDir, password, OverwriteMode::Overwrite, executor);
		Result res = extractor.Extract();

		BOOST_CHECK_EQUAL(command, expectedCommand);
		BOOST_CHECK(res.success);
	}
}

BOOST_AUTO_TEST_CASE(UnrarBuildsCorrectCommand)
{
	{
		std::string password = "secret";
		fs::path destWithSlash = outputDir / "";
#ifdef _WIN32
		std::string expectedCommand = "\"\"/bin/extractor\" x -y -ai -o+ -p\"secret\" \"" + archive.string() + "\" \"" + destWithSlash.string() + "\"" + Util::NULL_OUTPUT + "\"";
#else
		std::string expectedCommand = "\"/bin/extractor\" x -y -ai -o+ -p\"secret\" \"" + archive.string() + "\" \"" + destWithSlash.string() + "\"" + Util::NULL_OUTPUT;
#endif

		Unrar extractor(tool, archive, outputDir, password, OverwriteMode::Overwrite, executor);
		Result res = extractor.Extract();

		BOOST_CHECK_EQUAL(command, expectedCommand);
		BOOST_CHECK(res.success);
	}

	{
		std::string password = "";
		fs::path destWithSlash = outputDir / "";

#ifdef _WIN32
		std::string expectedCommand = "\"\"/bin/extractor\" x -y -ai -o+ -p- \"" + archive.string() + "\" \"" + destWithSlash.string() + "\"" + Util::NULL_OUTPUT + "\"";
#else
		std::string expectedCommand = "\"/bin/extractor\" x -y -ai -o+ -p- \"" + archive.string() + "\" \"" + destWithSlash.string() + "\"" + Util::NULL_OUTPUT;
#endif

		Unrar extractor(tool, archive, outputDir, password, OverwriteMode::Overwrite, executor);
		Result res = extractor.Extract();

		BOOST_CHECK_EQUAL(command, expectedCommand);
		BOOST_CHECK(res.success);
	}
}

BOOST_AUTO_TEST_CASE(SevenZipHandlesOverwriteMode)
{
	{
		SevenZip extractor(tool, archive, outputDir, "", OverwriteMode::Skip, executor);
		extractor.Extract();
		BOOST_CHECK_NE(command.find(" -aos"), std::string::npos);
	}

	{
		SevenZip extractor(tool, archive, outputDir, "", OverwriteMode::Overwrite, executor);
		extractor.Extract();
		BOOST_CHECK_NE(command.find(" -aoa"), std::string::npos);
	}

	{
		SevenZip extractor(tool, archive, outputDir, "", OverwriteMode::AutoRename, executor);
		extractor.Extract();
		BOOST_CHECK_NE(command.find(" -aou"), std::string::npos);
	}
}

BOOST_AUTO_TEST_CASE(UnrarHandlesOverwriteMode)
{
	{
		Unrar extractor(tool, archive, outputDir, "", OverwriteMode::Skip, executor);
		extractor.Extract();
		BOOST_CHECK_NE(command.find(" -o-"), std::string::npos);
	}

	{
		Unrar extractor(tool, archive, outputDir, "", OverwriteMode::Overwrite, executor);
		extractor.Extract();
		BOOST_CHECK_NE(command.find(" -o+"), std::string::npos);
	}

	{
		Unrar extractor(tool, archive, outputDir, "", OverwriteMode::AutoRename, executor);
		extractor.Extract();
		BOOST_CHECK_NE(command.find(" -or"), std::string::npos);
	}
}

BOOST_AUTO_TEST_CASE(ExtractorReturnsFailureOnNonZeroExitCode)
{
	exitCode = 2;

	SevenZip extractor(tool, archive, outputDir, "", OverwriteMode::Overwrite, executor);
	Result res = extractor.Extract();

	BOOST_CHECK_EQUAL(res.success, false);
}

BOOST_AUTO_TEST_CASE(ExtractorHandlesPathsWithSpaces)
{
	fs::path unpackWithSpaces = "my test Unpack.zip";
	fs::path outputWithSpaces = "my output folder";

	std::string expectedUnpackArg = "\"" + unpackWithSpaces.string() + "\"";
	std::string expectedOutputArg = "-o\"" + outputWithSpaces.string() + "\"";

	SevenZip extractor(tool, unpackWithSpaces, outputWithSpaces, "", OverwriteMode::Overwrite, executor);
	extractor.Extract();

	BOOST_CHECK_NE(command.find(expectedUnpackArg), std::string::npos);
	BOOST_CHECK_NE(command.find(expectedOutputArg), std::string::npos);

	fs::remove_all(outputWithSpaces);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(IsSupportedTests)
{
	BOOST_CHECK(SevenZip::IsSupported("Unpack.7z"));
	BOOST_CHECK(SevenZip::IsSupported("Unpack.zip"));
	BOOST_CHECK(SevenZip::IsSupported("Unpack.tar"));
	BOOST_CHECK(SevenZip::IsSupported("Unpack.gz"));
	BOOST_CHECK(SevenZip::IsSupported("Unpack.bz2"));

	BOOST_CHECK(SevenZip::IsSupported("Unpack.tar.gz"));
	BOOST_CHECK(SevenZip::IsSupported("Unpack.tgz"));
	BOOST_CHECK(SevenZip::IsSupported("Unpack.txz"));

	BOOST_CHECK(SevenZip::IsSupported("Unpack.7z.001"));

	BOOST_CHECK(SevenZip::IsSupported("Unpack.ZIP"));
	BOOST_CHECK(SevenZip::IsSupported("DATA.TXZ"));

	BOOST_CHECK(!SevenZip::IsSupported("Unpack.7z.002"));
	BOOST_CHECK(!SevenZip::IsSupported("document.docx"));
	BOOST_CHECK(!SevenZip::IsSupported("image.jpg"));
	BOOST_CHECK(!SevenZip::IsSupported("Unpack.zip.bak"));

	BOOST_CHECK(Unrar::IsSupported("Unpack.rar"));
	BOOST_CHECK(Unrar::IsSupported("Unpack.part1.rar"));
	BOOST_CHECK(Unrar::IsSupported("Unpack.part01.rar"));
	BOOST_CHECK(Unrar::IsSupported("Unpack.part001.rar"));

	BOOST_CHECK(Unrar::IsSupported("Unpack.RAR"));
	BOOST_CHECK(Unrar::IsSupported("DATA.Part1.Rar"));

	BOOST_CHECK(!Unrar::IsSupported("Unpack.r01"));
	BOOST_CHECK(!Unrar::IsSupported("Unpack.part2.rar"));
	BOOST_CHECK(!Unrar::IsSupported("Unpack.zip"));
	BOOST_CHECK(!Unrar::IsSupported("not_an_Unpack.txt"));
	BOOST_CHECK(!Unrar::IsSupported("Unpack.tar.gz"));
	BOOST_CHECK(!Unrar::IsSupported("Unpack.rar.bak"));
}
