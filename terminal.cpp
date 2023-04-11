#pragma once

#include <vector>
#include <string>

#include "lib.h"
#include "terminal.h"

using namespace std;

void Terminal::On() {
    this->state = TerminalState::Online;
}

TerminalState Terminal::GetStatus() {
    return this->state;
}

vector<Software> Terminal::GetSoftware() {
    return this->installed_software;
}

bool Terminal::InstallSoftware(Software sw) {
    this->installed_software.push_back(sw);
    return true;
}

bool Terminal::UpdateSoftware(Software oldsw, Software sw) {
    bool rs = this->RemoveSoftware(oldsw);
    bool is = this->InstallSoftware(sw);

    return rs && is;
}

bool Terminal::RemoveSoftware(Software sw) {
    vector<Software> *isw = &this->installed_software;
    isw->erase(std::remove(isw->begin(), isw->end(), sw), isw->end());
    return true;
}

vector<User> Terminal::GetUsers() {
    return this->users;
}

bool Terminal::AddUser(User user) {
    this->users.push_back(user);
    return true;
}

bool Terminal::ChangeUserRights(User olduser, User user) {
    bool rs = this->RemoveUser(olduser);
    bool is = this->AddUser(user);

    return rs && is;
}

bool Terminal::RemoveUser(User user) {
    vector<User> *us = &this->users;
    us->erase(std::remove(us->begin(), us->end(), user), us->end());
    return true;
}

bool Terminal::Login(const string &username) {
    bool result;
    for (auto &user: users) {
        if (user.username != username) continue;

        result = true;
        current_user = &user;
        break;
    }

    return result;
}

User *Terminal::GetLogginedUser() {
    return this->current_user;
}

void Terminal::Logout() {
    current_user = nullptr;
}

void Terminal::Off() {
    this->state = TerminalState::Offline;
}



