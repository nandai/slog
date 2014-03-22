grant all privileges on SequenceLogService.* to slog@localhost identified by 'DPdhE8iv1HQIe6nL' with grant option;
flush privileges;

create database SequenceLogService default character set utf8;
use             SequenceLogService;

create table user(
    id        int         primary key auto_increment,
    name      varchar(20) not null unique,
    password  varchar(64) not null,
    mail_addr varchar(256),
    version   tinyint     not null default 1,
    admin     tinyint     not null default 0);

insert into user (name, password, admin) values ('slog', 'RrtQzcEv7FQ1QaazVN+ZXHHAS/5F/MVuDUffTotnFKk=', 1);
