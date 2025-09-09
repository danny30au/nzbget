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

#ifndef APP_HEALTH_H
#define APP_HEALTH_H

#include <vector>
#include <unordered_map>
#include "HealthCheck.h"
#include "Options.h"
#include "Json.h"
#include "Xml.h"
#include "Checks.h"

namespace HealthCheck
{
	void HealthMonitor::RunChecks()
	{
		m_report = CheckUp();
	}

	HealthReport HealthMonitor::CheckUp()
	{
		HealthReport report;
		Sections sections;
		Report pathsSection;
		Report general;

		m_checks[Options::MAINDIR] = Checks::ComposeChecks(
			[]() { return Checks::CheckRequiredOption(Options::MAINDIR, g_Options->GetMainDir()); },
			[]() { return Checks::CheckRequiredDir(Options::MAINDIR, g_Options->GetMainDir()); },
			[]() { return Checks::Directory::Writable(Options::MAINDIR, g_Options->GetMainDir()); }
		);
		m_checks[Options::DESTDIR] = Checks::ComposeChecks(
			[]() { return Checks::CheckRequiredOption(Options::DESTDIR, g_Options->GetDestDir()); },
			[]() { return Checks::CheckRequiredDir(Options::DESTDIR, g_Options->GetDestDir()); },
			[]() { return Checks::Directory::Writable(Options::DESTDIR, g_Options->GetDestDir()); },
			[]() { return Checks::CheckValueUnique(Options::DESTDIR, g_Options->GetDestDir(),
				{
					{ Options::MAINDIR, g_Options->GetMainDir() }
				});
			}
		);
		m_checks[Options::INTERDIR] = Checks::ComposeChecks(
			[]() { return Checks::CheckInterDirConfiguration(*g_Options); },
			[]() { return Checks::CheckRequiredDir(Options::INTERDIR, g_Options->GetInterDir()); },
			[]() { return Checks::Directory::Writable(Options::INTERDIR, g_Options->GetInterDir()); },
			[]() { return Checks::CheckValueUnique(Options::INTERDIR, g_Options->GetInterDir(),
				{
					{ Options::MAINDIR, g_Options->GetMainDir() },
					{ Options::DESTDIR, g_Options->GetDestDir() }
				});
			}
		);
		m_checks[Options::NZBDIR] = Checks::ComposeChecks(
			[]() { return Checks::CheckRequiredOption(Options::NZBDIR, g_Options->GetNzbDir()); },
			[]() { return Checks::CheckRequiredDir(Options::NZBDIR, g_Options->GetNzbDir()); },
			[]() { return Checks::Directory::Writable(Options::NZBDIR, g_Options->GetNzbDir()); },
			[]() { return Checks::CheckValueUnique(Options::NZBDIR, g_Options->GetNzbDir(),
				{
					{ Options::MAINDIR, g_Options->GetMainDir() },
					{ Options::DESTDIR, g_Options->GetDestDir() },
					{ Options::INTERDIR, g_Options->GetInterDir() }
				});
			}
		);
		m_checks[Options::QUEUEDIR] = Checks::ComposeChecks(
			[]() { return Checks::CheckRequiredOption(Options::QUEUEDIR, g_Options->GetQueueDir()); },
			[]() { return Checks::CheckRequiredDir(Options::QUEUEDIR, g_Options->GetQueueDir()); },
			[]() { return Checks::Directory::Writable(Options::QUEUEDIR, g_Options->GetQueueDir()); },
			[]() { return Checks::CheckValueUnique(Options::QUEUEDIR, g_Options->GetQueueDir(),
				{
					{ Options::MAINDIR, g_Options->GetMainDir() },
					{ Options::DESTDIR, g_Options->GetDestDir() },
					{ Options::INTERDIR, g_Options->GetInterDir() },
					{ Options::NZBDIR, g_Options->GetNzbDir() }
				});
			}
		);
		m_checks[Options::WEBDIR] = Checks::ComposeChecks(
			[]() { return Checks::CheckRequiredDir(Options::WEBDIR, g_Options->GetWebDir()); },
			[]() { return Checks::Directory::Readable(Options::WEBDIR, g_Options->GetWebDir()); },
			[]() { return Checks::CheckValueUnique(Options::WEBDIR, g_Options->GetWebDir(),
				{
					{ Options::MAINDIR, g_Options->GetMainDir() },
					{ Options::DESTDIR, g_Options->GetDestDir() },
					{ Options::INTERDIR, g_Options->GetInterDir() },
					{ Options::QUEUEDIR, g_Options->GetQueueDir() },
					{ Options::NZBDIR, g_Options->GetNzbDir() },
					{ Options::TEMPDIR, g_Options->GetTempDir() }
				});
			}
		);
		m_checks[Options::TEMPDIR] = Checks::ComposeChecks(
			[]() { return Checks::CheckRequiredOption(Options::TEMPDIR, g_Options->GetTempDir()); },
			[]() { return Checks::CheckRequiredDir(Options::TEMPDIR, g_Options->GetTempDir()); },
			[]() { return Checks::Directory::Writable(Options::TEMPDIR, g_Options->GetTempDir()); },
			[]() { return Checks::CheckValueUnique(Options::TEMPDIR, g_Options->GetTempDir(),
				{
					{ Options::MAINDIR, g_Options->GetMainDir() },
					{ Options::DESTDIR, g_Options->GetDestDir() },
					{ Options::INTERDIR, g_Options->GetInterDir() },
					{ Options::QUEUEDIR, g_Options->GetQueueDir() },
					{ Options::NZBDIR, g_Options->GetNzbDir() }
				});
			}
		);
		m_checks[Options::SCRIPTDIR] = Checks::ComposeChecks(
			[]() { return Checks::CheckRequiredOption(Options::SCRIPTDIR, g_Options->GetScriptDir()); },
			[]() { return Checks::CheckRequiredDir(Options::SCRIPTDIR, g_Options->GetScriptDir()); },
			[]() { return Checks::Directory::Writable(Options::SCRIPTDIR, g_Options->GetScriptDir()); },
			[]() { return Checks::CheckValueUnique(Options::SCRIPTDIR, g_Options->GetScriptDir(),
				{
					{ Options::MAINDIR, g_Options->GetMainDir() },
					{ Options::DESTDIR, g_Options->GetDestDir() },
					{ Options::INTERDIR, g_Options->GetInterDir() },
					{ Options::QUEUEDIR, g_Options->GetQueueDir() },
					{ Options::NZBDIR, g_Options->GetNzbDir() },
					{ Options::WEBDIR, g_Options->GetWebDir() }
				});
			}
		);
		m_checks[Options::CONFIGTEMPLATE] = Checks::ComposeChecks(
			[]() { return Checks::CheckRequiredOption(Options::CONFIGTEMPLATE, g_Options->GetConfigTemplate()); },
			[]() { return Checks::File::Exists(Options::CONFIGTEMPLATE, g_Options->GetConfigTemplate()); },
			[]() { return Checks::File::Readable(Options::CONFIGTEMPLATE, g_Options->GetConfigTemplate()); },
			[]() { return Checks::File::Writable(Options::CONFIGTEMPLATE, g_Options->GetConfigTemplate()); }
		);
		m_checks[Options::LOGFILE] = Checks::ComposeChecks(
			[]() { return Checks::CheckLoggingConfiguration(*g_Options); }
		);
		m_checks[Options::CERTSTORE] = Checks::ComposeChecks(
			[]() { return Checks::CheckCertStoreConfiguration(*g_Options); }
		);
		m_checks[Options::REQUIREDDIR] = Checks::ComposeChecks(
			[]() { return Check::Ok(); }
		);
#ifndef _WIN32
		m_checks[Options::LOCKFILE] = Checks::ComposeChecks(
			[]() { return Checks::CheckLockFileConfiguration(*g_Options); }
		);
#endif	
		pathsSection[Options::MAINDIR] = m_checks[Options::MAINDIR]();
		pathsSection[Options::DESTDIR] = m_checks[Options::DESTDIR]();
		pathsSection[Options::INTERDIR] = m_checks[Options::INTERDIR]();
		pathsSection[Options::NZBDIR] = m_checks[Options::NZBDIR]();
		pathsSection[Options::QUEUEDIR] = m_checks[Options::QUEUEDIR]();
		pathsSection[Options::TEMPDIR] = m_checks[Options::TEMPDIR]();
		pathsSection[Options::WEBDIR] = m_checks[Options::WEBDIR]();
		pathsSection[Options::SCRIPTDIR] = m_checks[Options::SCRIPTDIR]();
		pathsSection[Options::LOGFILE] = m_checks[Options::LOGFILE]();
		pathsSection[Options::CERTSTORE] = m_checks[Options::CERTSTORE]();
		pathsSection[Options::REQUIREDDIR] = m_checks[Options::REQUIREDDIR]();
		pathsSection[Options::CONFIGTEMPLATE] = m_checks[Options::CONFIGTEMPLATE]();
#ifndef _WIN32
		pathsSection[Options::LOCKFILE] = m_checks[Options::LOCKFILE]();
#endif
		for( const auto&[opt, check] : pathsSection)
		{
			if (!check.IsOk())
			{
				general[opt] = check;
			}
		}
		sections.push_back({ "Paths", std::move(pathsSection) });

		report.sections.swap(sections);
		report.general.swap(general);

		return report;
	}

	std::string ToJsonStr(const HealthReport& report)
	{
		Json::JsonObject reportJson;
		Json::JsonObject sectionsJson;
		Json::JsonObject generalJson;

		for (auto& [sectionName, section] : report.sections)
		{
			Json::JsonObject sectionJson;
			for (const auto& [name, check] : section)
			{
				sectionJson[name] = ToJson(check);
			}

			sectionsJson[sectionName] = std::move(sectionJson);
		}

		for (const auto& [name, check] : report.general)
		{
			generalJson[name] = ToJson(check);
		}

		reportJson["Sections"] = std::move(sectionsJson);
		reportJson["General"] = std::move(generalJson);

		return Json::serialize(reportJson);
	}

	std::string ToXmlStr(const HealthReport& report)
	{
		return "";
	}
}

#endif
