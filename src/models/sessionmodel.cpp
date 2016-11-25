/*
    Copyright (C) 2015 by Elvis Angelaccio <elvis.angelaccio@kde.org>

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

#include "sessionmodel.h"

#include <KLocalizedString>

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

SessionModel::SessionModel(QObject *parent) : QAbstractTableModel(parent)
{
    QFile saveFile {QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QLatin1String("/sessions.json")};
    if (saveFile.exists()) {
        if (saveFile.open(QIODevice::ReadOnly)) {
            auto saveData = saveFile.readAll();
            auto saveDoc = QJsonDocument::fromJson(saveData);
            read(saveDoc.object());
        } else {
            qDebug().nospace() << "cannot open " << saveFile.fileName() << ": " << saveFile.errorString();
        }
    }

    connect(this, &QAbstractTableModel::dataChanged, this, &SessionModel::slotWrite);
    connect(this, &QAbstractTableModel::rowsInserted, this, &SessionModel::slotWrite);
    connect(this, &QAbstractTableModel::rowsRemoved, this, &SessionModel::slotWrite);
}

int SessionModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return m_roles.count();
}

int SessionModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return m_sessionList.size();
}

QVariant SessionModel::data(const QModelIndex& index, int role) const
{
    if (not index.isValid()) {
        return QVariant::Invalid;
    }

    if (index.row() >= m_sessionList.size() || index.row() < 0) {
        return QVariant::Invalid;
    }

    if (role == Qt::DisplayRole) {
        return data(index, Qt::UserRole + index.column());
    }

    switch (static_cast<Roles>(role)) {
    case Roles::SessionIdRole:
        return QString::number(index.row() + 1);
    case Roles::NameRole:
        return m_sessionList.at(index.row()).name();
    case Roles::DateRole:
        return m_sessionList.at(index.row()).date();
    case Roles::NoteRole:
        return m_sessionList.at(index.row()).note();
    }

    if (role == Qt::EditRole && index.column() == columnForRole(Roles::NameRole)) {
        // prevent the disappear of the old value when double-clicking the item
        return m_sessionList.at(index.row()).name();
    }

    if (role == Qt::EditRole && index.column() == columnForRole(Roles::NoteRole)) {
        return m_sessionList.at(index.row()).note();
    }

    return QVariant::Invalid;
}


QVariant SessionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole or orientation != Qt::Horizontal)
        return QVariant::Invalid;

    switch (roleForColumn(section)) {
    case Roles::SessionIdRole:
        return i18nc("session number", "Session #");
    case Roles::NameRole:
        return i18n("Name");
    case Roles::DateRole:
        return i18n("Date");
    case Roles::NoteRole:
        return i18n("Note");
    }

    return QVariant::Invalid;
}

bool SessionModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (not index.isValid() or role != Qt::EditRole)
        return false;

    if (index.column() == columnForRole(Roles::NameRole)) {
        if (value.toString().isEmpty())
            return false;

        m_sessionList[index.row()].setName(value.toString());
        emit dataChanged(index, index);

        return true;
    }

    if (index.column() == columnForRole(Roles::NoteRole)) {
        m_sessionList[index.row()].setNote(value.toString());
        emit dataChanged(index, index);

        return true;
    }

    return false;
}

Qt::ItemFlags SessionModel::flags(const QModelIndex& index) const
{
    if (not index.isValid())
        return Qt::ItemIsEnabled;

    if (isEditable(index))
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;

    return QAbstractTableModel::flags(index);
}

bool SessionModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (row < 0 || row + count - 1 >= m_sessionList.size())
        return false;

    beginRemoveRows(parent, row, row + count - 1);

    for (int i = 0; i < count; i++) {
        m_sessionList.removeAt(row);
    }

    endRemoveRows();

    return true;
}

const Session& SessionModel::at(int sessionIndex)
{
    return m_sessionList.at(sessionIndex);
}

void SessionModel::append(const Session& session)
{
    beginInsertRows(QModelIndex(), m_sessionList.size(), m_sessionList.size());    // i.e. append the new row at the table end
    m_sessionList.append(session);
    endInsertRows();
}

void SessionModel::update(const Session& session)
{
    int i = m_sessionList.indexOf(session);

    if (i == -1)
        return;

    m_sessionList[i] = session;
    m_sessionList[i].setDate(QDateTime::currentDateTime());

    emit dataChanged({}, {});
}

bool SessionModel::isEmpty() const
{
    return m_sessionList.isEmpty();
}

bool SessionModel::isEditable(const QModelIndex& index) const
{
    auto role = roleForColumn(index.column());
    return role == Roles::NameRole || role == Roles::NoteRole;
}

void SessionModel::read(const QJsonObject& json)
{
    const auto sessions = json[QStringLiteral("sessions")].toArray();
    for (const auto& session : sessions) {
        append(Session::fromJson(session.toObject()));
    }
}

void SessionModel::slotWrite()
{
    auto sessions = QJsonArray {};
    for (const auto& session : m_sessionList) {
        auto object = QJsonObject {};
        session.write(object);
        sessions.append(object);
    }

    auto json = QJsonObject {};
    json[QStringLiteral("sessions")] = sessions;

    QFile saveFile {QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QLatin1String("/sessions.json")};
    saveFile.open(QIODevice::WriteOnly | QIODevice::Truncate);

    auto saveDoc = QJsonDocument {json};
            saveFile.write(saveDoc.toJson());
}

int SessionModel::columnForRole(SessionModel::Roles role) const
{
    return m_roles.indexOf(role);
}

SessionModel::Roles SessionModel::roleForColumn(int column) const
{
    return m_roles.at(column);
}
