/*
    Copyright (C) 2014 by Elvis Angelaccio <elvis.angelaccio@kdemail.net>

    This file is part of Kronometer.

    Kronometer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    Kronometer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Kronometer.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "digitdisplay.h"
#include "timedisplay.h"

#include <KLocalizedString>

#include <QBoxLayout>
#include <QLabel>

namespace
{
    const QString FRAME_STYLE = QStringLiteral("QFrame {background-clip: content; background: %1; color: %2}");
}

TimeDisplay::TimeDisplay(QWidget *parent) : QWidget(parent), m_displayTime(0, 0)
{
    auto displayLayout = new QHBoxLayout(this);

    m_hourFrame = new QFrame(this);
    m_minFrame = new QFrame(this);
    m_secFrame = new QFrame(this);
    m_fracFrame = new QFrame(this);
    m_hourFrame->setFrameShape(QFrame::StyledPanel);
    m_hourFrame->setFrameShadow(QFrame::Sunken);
    m_minFrame->setFrameShape(QFrame::StyledPanel);
    m_minFrame->setFrameShadow(QFrame::Sunken);
    m_secFrame->setFrameShape(QFrame::StyledPanel);
    m_secFrame->setFrameShadow(QFrame::Sunken);
    m_fracFrame->setFrameShape(QFrame::StyledPanel);
    m_fracFrame->setFrameShadow(QFrame::Sunken);

    auto hourLayout = new QVBoxLayout(m_hourFrame);
    auto minLayout = new QVBoxLayout(m_minFrame);
    auto secLayout = new QVBoxLayout(m_secFrame);
    auto fracLayout = new QVBoxLayout(m_fracFrame);

    m_hourHeader = new QLabel(m_hourFrame);
    m_minHeader = new QLabel(m_minFrame);
    m_secHeader = new QLabel(m_secFrame);
    m_fracHeader = new QLabel(m_fracFrame);
    m_hourHeader->setAlignment(Qt::AlignCenter);
    m_minHeader->setAlignment(Qt::AlignCenter);
    m_secHeader->setAlignment(Qt::AlignCenter);
    m_fracHeader->setAlignment(Qt::AlignCenter);
    m_hourHeader->setText(i18n("Hours"));
    m_minHeader->setText(i18n("Minutes"));
    m_secHeader->setText(i18n("Seconds"));
    m_fracHeader->setText(i18n("Hundredths"));
    m_hourHeader->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_minHeader->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_secHeader->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_fracHeader->setTextInteractionFlags(Qt::TextSelectableByMouse);

    m_hourDisplay = new DigitDisplay(m_hourFrame);
    m_minDisplay = new DigitDisplay(m_minFrame);
    m_secDisplay = new DigitDisplay(m_secFrame);
    m_fracDisplay = new DigitDisplay(m_fracFrame);

    m_hourDisplay->setDigitCounter(DigitDisplay::TwoDigits);
    m_minDisplay->setDigitCounter(DigitDisplay::TwoDigits);
    m_secDisplay->setDigitCounter(DigitDisplay::TwoDigits);

    m_hourDisplay->showDigits(m_currentFormat.formatHour(m_displayTime));
    m_minDisplay->showDigits(m_currentFormat.formatMin(m_displayTime));
    m_secDisplay->showDigits(m_currentFormat.formatSec(m_displayTime));
    m_fracDisplay->showDigits(m_currentFormat.formatSecFrac(m_displayTime));

    hourLayout->addWidget(m_hourHeader);
    hourLayout->addWidget(m_hourDisplay);
    minLayout->addWidget(m_minHeader);
    minLayout->addWidget(m_minDisplay);
    secLayout->addWidget(m_secHeader);
    secLayout->addWidget(m_secDisplay);
    fracLayout->addWidget(m_fracHeader);
    fracLayout->addWidget(m_fracDisplay);

    QMargins margins = displayLayout->contentsMargins();
    margins.setTop(0);
    margins.setBottom(0);

    displayLayout->setContentsMargins(margins);
    displayLayout->addWidget(m_hourFrame);
    displayLayout->addWidget(m_minFrame);
    displayLayout->addWidget(m_secFrame);
    displayLayout->addWidget(m_fracFrame);
}

void TimeDisplay::setTimeFormat(const TimeFormat& format)
{
    m_defaultFormat = format;
    m_currentFormat = format;
    updateTimeFormat();
}

void TimeDisplay::setHourFont(const QFont& font)
{
    m_hourDisplay->setFont(font);
    updateWidth();
}

void TimeDisplay::setMinFont(const QFont& font)
{
    m_minDisplay->setFont(font);
    updateWidth();
}

void TimeDisplay::setSecFont(const QFont& font)
{
    m_secDisplay->setFont(font);
    updateWidth();
}

void TimeDisplay::setFracFont(const QFont& font)
{
    m_fracDisplay->setFont(font);
    updateWidth();
}

void TimeDisplay::setBackgroundColor(const QColor& color)
{
    m_backgroundColor = color;

    m_hourFrame->setStyleSheet(FRAME_STYLE.arg(m_backgroundColor.name(), m_textColor.name()));
    m_minFrame->setStyleSheet(FRAME_STYLE.arg(m_backgroundColor.name(), m_textColor.name()));
    m_secFrame->setStyleSheet(FRAME_STYLE.arg(m_backgroundColor.name(), m_textColor.name()));
    m_fracFrame->setStyleSheet(FRAME_STYLE.arg(m_backgroundColor.name(), m_textColor.name()));
}

void TimeDisplay::setTextColor(const QColor& color)
{
    m_textColor = color;

    m_hourFrame->setStyleSheet(FRAME_STYLE.arg(m_backgroundColor.name(), m_textColor.name()));
    m_minFrame->setStyleSheet(FRAME_STYLE.arg(m_backgroundColor.name(), m_textColor.name()));
    m_secFrame->setStyleSheet(FRAME_STYLE.arg(m_backgroundColor.name(), m_textColor.name()));
    m_fracFrame->setStyleSheet(FRAME_STYLE.arg(m_backgroundColor.name(), m_textColor.name()));
}

void TimeDisplay::showHeaders(bool show)
{
    m_hourHeader->setVisible(show);
    m_minHeader->setVisible(show);
    m_secHeader->setVisible(show);
    m_fracHeader->setVisible(show);
}

QString TimeDisplay::currentTime()
{
    m_currentFormat.showDividers(true);
    auto currentTime = m_currentFormat.format(m_displayTime);
    m_currentFormat.showDividers(false);

    return currentTime;
}

void TimeDisplay::slotTime(int time)
{
    m_displayTime.setHMS(time / MSECS_PER_HOUR,
                        (time % MSECS_PER_HOUR) / MSECS_PER_MIN,
                        (time / MSECS_PER_SEC) % 60,
                        time % MSECS_PER_SEC);

    updateTimer();
}

void TimeDisplay::slotReset()
{
    if (m_currentFormat == m_defaultFormat)
        return;

    setTimeFormat(m_defaultFormat);
}

void TimeDisplay::updateTimer()
{
    if (m_currentFormat.isHourEnabled()) {
        m_hourDisplay->showDigits(m_currentFormat.formatHour(m_displayTime));
    }
    else if (m_displayTime.hour() >= 1) {
        m_currentFormat.overrideHours();
        updateTimeFormat();
    }

    if (m_currentFormat.isMinEnabled()) {
        m_minDisplay->showDigits(m_currentFormat.formatMin(m_displayTime));
    }
    else if (m_displayTime.minute() >= 1) {
        m_currentFormat.overrideMin();
        updateTimeFormat();
    }

    m_secDisplay->showDigits(m_currentFormat.formatSec(m_displayTime));

    if (m_currentFormat.isFractionEnabled()) {
        m_fracDisplay->showDigits(m_currentFormat.formatSecFrac(m_displayTime));
    }
}

void TimeDisplay::updateWidth()
{
    int width = qMax(qMax(m_hourDisplay->width(), m_minDisplay->width()), qMax(m_secDisplay->width(), m_fracDisplay->width()));

    width = width + (width * 20 / 100); // 20% as padding, i.e. 10% as right padding and 10% as left padding

    m_hourFrame->setMinimumWidth(width);
    m_minFrame->setMinimumWidth(width);
    m_secFrame->setMinimumWidth(width);
    m_fracFrame->setMinimumWidth(width);
}

void TimeDisplay::updateTimeFormat()
{
    m_hourFrame->setVisible(m_currentFormat.isHourEnabled());
    m_minFrame->setVisible(m_currentFormat.isMinEnabled());
    m_fracFrame->setVisible(m_currentFormat.isFractionEnabled());

    if (m_currentFormat.isFractionEnabled()) {
        switch (m_currentFormat.secondFractions()) {
        case TimeFormat::UpToTenths:
            m_fracHeader->setText(i18n("Tenths"));
            m_fracDisplay->setDigitCounter(DigitDisplay::OneDigit);
            break;
        case TimeFormat::UpToHundredths:
            m_fracHeader->setText(i18n("Hundredths"));
            m_fracDisplay->setDigitCounter(DigitDisplay::TwoDigits);
            break;
        case TimeFormat::UpToMilliseconds:
            m_fracHeader->setText(i18n("Milliseconds"));
            m_fracDisplay->setDigitCounter(DigitDisplay::ThreeDigits);
            break;
        default:
            break;
        }
    }

    updateTimer();
}

