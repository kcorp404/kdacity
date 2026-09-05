/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "startupscenario.h"

#include <QDate>

#include "framework/global/log.h"
#include "framework/global/settings.h"
#include "framework/global/types/uri.h"

#include "appshell/appshelltypes.h"

using namespace au::appshell;
using namespace muse::actions;
using namespace au::project;


static const muse::UriQuery FIRST_LAUNCH_SETUP_URI("audacity://firstLaunchSetup?floating=true");
static const muse::Uri ALPHA_WELCOME_POPUP("audacity://alphaWelcomePopup");
static const muse::Uri HOME_URI("audacity://home");
static const muse::Uri PROJECT_URI("audacity://project");

static StartupModeType modeTypeFromString(const std::string& str)
{
    if ("start-empty" == str) {
        return StartupModeType::StartEmpty;
    }

    if ("continue-last" == str) {
        return StartupModeType::ContinueLastSession;
    }

    if ("start-with-new" == str) {
        return StartupModeType::StartWithNewProject;
    }

    if ("start-with-file" == str) {
        return StartupModeType::StartWithProject;
    }

    return StartupModeType::StartEmpty;
}

void StartupScenario::setStartupType(const std::optional<std::string>& type)
{
    m_startupTypeStr = type ? type.value() : "";
}

bool StartupScenario::isStartWithNewFileAsSecondaryInstance() const
{
    if (m_startupProjectFile.isValid()) {
        return false;
    }

    if (!m_startupTypeStr.empty()) {
        return modeTypeFromString(m_startupTypeStr) == StartupModeType::StartWithNewProject;
    }

    return false;
}

const ProjectFile& StartupScenario::startupProjectFile() const
{
    return m_startupProjectFile;
}

void StartupScenario::setStartupProjectFile(const std::optional<ProjectFile>& file)
{
    m_startupProjectFile = file ? file.value() : ProjectFile();
}

const muse::io::paths_t& StartupScenario::startupMediaFiles() const
{
    return m_startupMediaFiles;
}

void StartupScenario::setStartupMediaFiles(const muse::io::paths_t& files)
{
    m_startupMediaFiles = files;
}

bool StartupScenario::removeMediaFilesAfterImport() const
{
    return m_removeMediaFilesAfterImport;
}

void StartupScenario::setRemoveMediaFilesAfterImport(bool remove)
{
    m_removeMediaFilesAfterImport = remove;
}

void StartupScenario::setStartupUrl(const QString& url)
{
    if (m_startupCompleted && !url.isEmpty()) {
        dispatcher()->dispatch("open-url", ActionData::make_arg1<QString>(url));
        return;
    }
    m_startupUrl = url;
}

muse::async::Promise<muse::Ret> StartupScenario::runOnSplashScreen()
{
    return muse::async::make_promise<muse::Ret>([this](auto resolve, auto) {
        const muse::Ret ret = muse::make_ret(muse::Ret::Code::Ok);
        return resolve(ret);
    });
}

void StartupScenario::runAfterSplashScreen()
{
    TRACEFUNC;

    if (m_startupCompleted) {
        return;
    }

    m_startupCompleted = true;

    StartupModeType modeType = resolveStartupModeType();
    const bool canOverrideStartupMode = multiwindowsProvider()->isFirstWindow() && !hasExplicitStartupTarget();
    if (canOverrideStartupMode && sessionsManager()->hasProjectsForRestore()) {
        modeType = StartupModeType::Recovery;
    }
    if (canOverrideStartupMode && !configuration()->hasCompletedFirstLaunchSetup()) {
        modeType = StartupModeType::FirstLaunch;
    }

    const muse::Uri startupUri = startupPageUri(modeType);

    auto promise = interactive()->open(startupUri);
    promise.onResolve(this, [this, modeType](const muse::Val&) {
        effectsProviderInitializer()->callAfterSplashScreen();

        onStartupPageOpened(modeType);
    });
}

bool StartupScenario::startupCompleted() const
{
    return m_startupCompleted;
}

bool StartupScenario::hasExplicitStartupTarget() const
{
    return m_startupProjectFile.isValid() || !m_startupMediaFiles.empty() || !m_startupUrl.isEmpty();
}

StartupModeType StartupScenario::resolveStartupModeType() const
{
    if (m_startupProjectFile.isValid()) {
        return StartupModeType::StartWithProject;
    }

    if (!m_startupMediaFiles.empty()) {
        return StartupModeType::StartEmpty;
    }

    if (!m_startupTypeStr.empty()) {
        return modeTypeFromString(m_startupTypeStr);
    }

    return configuration()->startupModeType();
}

void StartupScenario::onStartupPageOpened(StartupModeType modeType)
{
    TRACEFUNC;

    // Kdacity ships no welcome/promo carousel and no update check. The only
    // thing left to run after the splash is the first-launch setup, which is
    // entirely local (theme + audio device).
    if (!configuration()->hasCompletedFirstLaunchSetup()) {
        interactive()->open(FIRST_LAUNCH_SETUP_URI);
    }
}

muse::Uri StartupScenario::startupPageUri(StartupModeType modeType) const
{
    switch (modeType) {
    case StartupModeType::StartEmpty:
    case StartupModeType::StartWithNewProject:
    case StartupModeType::Recovery:
        return HOME_URI;
    case StartupModeType::StartWithProject:
    case StartupModeType::ContinueLastSession:
    case StartupModeType::FirstLaunch:
        return PROJECT_URI;
    }

    return HOME_URI;
}

void StartupScenario::openProject(const ProjectFile& file)
{
    dispatcher()->dispatch("file-open", ActionData::make_arg2<QUrl, QString>(file.url, file.displayNameOverride));
}

void StartupScenario::restoreLastSession()
{
    auto promise = interactive()->question(muse::trc("appshell", "The previous session quit unexpectedly."),
                                           muse::trc("appshell", "Do you want to restore the session?"),
                                           { muse::IInteractive::Button::No, muse::IInteractive::Button::Yes },
                                           muse::IInteractive::Button::NoButton, {},
                                           muse::trc("appshell", "Restore session"));

    promise.onResolve(this, [this](const muse::IInteractive::Result& res) {
        if (res.isButton(muse::IInteractive::Button::Yes)) {
            sessionsManager()->restore();
        } else {
            sessionsManager()->reset();
        }
    });
}

