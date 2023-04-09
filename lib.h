#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <set>

#include "libs/nlohmann/json.h"

namespace nh = nlohmann;
using namespace std;

enum class TerminalState {
    Online,
    Offline,
    Undefined
};

constexpr const char *TerminalStateToString(TerminalState e) {
    switch (e) {
        case TerminalState::Online:
            return "Online";
        case TerminalState::Offline:
            return "Offline";
        case TerminalState::Undefined:
            return "Undefined";
    }
}

const size_t MAX_SEND_SIZE = 1100;

struct Message {
    long mtype;
    char mtext[MAX_SEND_SIZE];
};

struct Software {
    string name;
    string version;

    bool operator==(Software const &rhs) const {
        return this->name == rhs.name && this->version == rhs.version;
    }
};


struct User {
    string username;
    vector<Software> allowed_software;

    bool operator==(User const &rhs) const {
        return this->username == rhs.username && this->username == rhs.username;
    }
};

struct Request {
    string route;

    User oldUser;
    User user;

    Software oldSoftware;
    Software software;
};

struct Response {
    int status = 200;

    TerminalState state;
    vector<User> users;
    vector<Software> software;
    User *current_loggined_user;
};

namespace nlohmann {
    template<>
    struct adl_serializer<Software> {
        static Software from_json(const json &j) {
            Software sw;
            sw.name = j.at("id").get<string>();
            sw.version = j.at("name").get<string>();
            return sw;
        }

        static void to_json(json &j, Software t) {
            j["name"] = t.name;
            j["version"] = t.version;
        }
    };

    template<>
    struct adl_serializer<User> {
        static User from_json(const json &j) {
            User u;
            u.username = j.at("username").get<string>();
            u.allowed_software = j.at("allowed_software").get<vector<Software>>();
            return u;
        }

        static void to_json(json &j, User t) {
            j["username"] = t.username;
            j["allowed_software"] = t.allowed_software;
        }
    };

    template<>
    struct adl_serializer<Request> {
        static Request from_json(const json &j) {
            Request sw;
            sw.route = j.at("id").get<string>();
            sw.oldUser = j.at("oldUser").get<User>();
            sw.user = j.at("user").get<User>();
            sw.oldSoftware = j.at("oldSoftware").get<Software>();
            sw.software = j.at("software").get<Software>();
            return sw;
        }

        static void to_json(json &j, Request t) {
            j["route"] = t.route;
            j["oldUser"] = t.oldUser;
            j["user"] = t.user;
            j["software"] = t.oldSoftware;
            j["oldSoftware"] = t.software;
        }
    };

    template<>
    struct adl_serializer<Response> {
        static Response from_json(const json &j) {
            Response u;
            u.status = j.at("status").get<int>();
            u.state = j.at("state").get<TerminalState>();
            u.users = j.at("users").get<vector<User>>();
            u.software = j.at("software").get<vector<Software>>();
            return u;
        }

        static void to_json(json &j, Response t) {
            j["status"] = t.status;
            j["state"] = t.state;
            j["users"] = t.users;
            j["software"] = t.software;

            // TODO: refactor
            // j["username"] = t.current_loggined_user;
        }
    };
}
