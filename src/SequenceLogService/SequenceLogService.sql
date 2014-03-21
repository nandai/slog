create table user(
    id        integer primary key autoincrement,
    name      varchar not null unique,
    password  varchar not null,
    mail_addr varchar,
    version   int     not null default 1,
    admin     int     not null default 0);

insert into user (name, password, admin) values ('slog', 'RrtQzcEv7FQ1QaazVN+ZXHHAS/5F/MVuDUffTotnFKk=', 1);
