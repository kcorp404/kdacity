/*
* Audacity: A Digital Audio Editor
*/
#pragma once

#include "actions/actionable.h"
#include "uicomponents/qml/Muse/UiComponents/abstracttoolbarmodel.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "context/iuicontextresolver.h"

namespace au::projectscene {
class ProjectToolBarModel : public muse::uicomponents::AbstractToolBarModel, public muse::actions::Actionable
{
    Q_OBJECT

    Q_PROPERTY(bool isCompactMode READ isCompactMode WRITE setIsCompactMode NOTIFY isCompactModeChanged)

    muse::ContextInject<muse::actions::IActionsDispatcher> dispatcher { this };
    muse::ContextInject<context::IGlobalContext> context { this };
    muse::ContextInject<context::IUiContextResolver> uicontextResolver { this };

public:
    Q_INVOKABLE void load() override;

    bool isCompactMode() const;
    void setIsCompactMode(bool isCompactMode);

signals:
    void openAudioSetupContextMenu();
    void isCompactModeChanged();

private:
    void onActionsStateChanges(const muse::actions::ActionCodeList& codes) override;

    bool m_loaded = false;
    bool m_isCompactMode = false;
};
}
