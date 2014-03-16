create table user(
    id       integer primary key autoincrement,
    name     varchar not null,
    password varchar not null,
    admin    int     not null default 0);

insert into user (name, password, admin) values ('slog', 'JsAapLjJDQhMt2bbWtVv4GFTq3o=', 1);
