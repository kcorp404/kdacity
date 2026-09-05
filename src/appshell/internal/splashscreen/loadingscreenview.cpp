/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "loadingscreenview.h"

#include <QApplication>
#include <QPainter>
#include <QScreen>
#include <QSvgRenderer>

#include "translation.h"

#include "muse_framework_config.h"

using namespace au::appshell;
using namespace muse;

static const QString imagePath(":/resources/LoadingScreen.svg");

static constexpr QSize loadingScreenSize(800, 380);

static const QColor messageColor("#99FFFFFF");
static constexpr QRectF messageRect(loadingScreenSize.width() / 2, 269, 0, 0);

static const QColor wordmarkColor("#FFFFFF");

//! The "K" monogram drawn by LoadingScreen.svg occupies x 38..126, y 112..210.
//! The wordmark sits to its right and is vertically centred on it.
static constexpr QRectF wordmarkRect(150, 161, 0, 0);
static constexpr QRectF taglineRect(152, 196, 0, 0);

static constexpr QRectF versionRect(38, 322, 0, 0);

LoadingScreenView::LoadingScreenView(const muse::modularity::ContextPtr& ctx, QWidget* parent)
    : QWidget(parent), muse::Contextable(ctx),
    m_backgroundRenderer(new QSvgRenderer(imagePath, this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    resize(loadingScreenSize);

    m_message = qtrc("appshell", "Loading…\u200e");
}

bool LoadingScreenView::event(QEvent* event)
{
    if (event->type() == QEvent::Paint) {
        QPainter painter(this);
        painter.setLayoutDirection(layoutDirection());
        draw(&painter);
    }

    return QWidget::event(event);
}

void LoadingScreenView::draw(QPainter* painter)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    // Draw background
    m_backgroundRenderer->render(painter);

    // Draw message
    QFont font(QString::fromStdString(uiConfiguration()->fontFamily()));
    font.setPixelSize(uiConfiguration()->fontSize());

    painter->setFont(font);

    QPen pen(messageColor);
    painter->setPen(pen);

    // painter->drawText(messageRect, Qt::AlignTop | Qt::AlignHCenter | Qt::TextDontClip, m_message);

    //! TODO AU4
    Qt::AlignmentFlag alignment = Qt::AlignLeft;
    // Qt::AlignmentFlag alignment = languagesService()->currentLanguage().direction == Qt::RightToLeft
    //                               ? Qt::AlignLeft : Qt::AlignRight;

    // Draw the app name next to the monogram
    QFont wordmarkFont(font);
    wordmarkFont.setPixelSize(46);
    wordmarkFont.setWeight(QFont::DemiBold);
    painter->setFont(wordmarkFont);

    pen.setColor(wordmarkColor);
    painter->setPen(pen);

    //! Deliberately not application()->title(): that appends " Development" on
    //! unstable builds, which overruns the panel. The build channel is shown on
    //! the version line at the bottom instead.
    painter->drawText(wordmarkRect, Qt::AlignVCenter | alignment | Qt::TextDontClip,
                      QStringLiteral(MUSE_APP_NAME_HUMAN_READABLE));

    // Draw the tagline
    painter->setFont(font);

    pen.setColor(messageColor);
    painter->setPen(pen);

    painter->drawText(taglineRect, Qt::AlignVCenter | alignment | Qt::TextDontClip,
                      //: Tagline shown under the app name on the splash screen
                      qtrc("appshell", "Kept offline and private."));

    // Draw version number
    pen.setColor(uiConfiguration()->currentTheme().extra["logo_main_color"].value<QColor>());
    painter->setPen(pen);

    QString versionText = qtrc("appshell", "Version %1").arg(application()->version().major());
    if (application()->unstable()) {
        versionText += QStringLiteral(" · Development");
    }

    painter->drawText(versionRect, Qt::AlignBottom | alignment | Qt::TextDontClip, versionText);
}
