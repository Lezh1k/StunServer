# StunServer
Базовая реализация STUN-сервера.
Поддерживает методы binding и success response. Использует 2 адреса и 2 порта для определения типа NAT.

Для установки нужно сделать следующее:

1. make
2. sudo make install 
Исполняемый файл будет в `/usr/local/bin/sedi_stun`
3. Отредактировать файл sedi_stun.service (указать в параметрах реальные адреса своего сервера. обязательно 2 ip адреса)
4. Скопировать sedi_stun.service в /etc/systemd/system/
5. Использовать systemctl для управления. (systemctl start/stop or systemctl enable/disable or systemctl status sedi_stun.service)
6. Для просмотра логов использовать journalctl /usr/local/bin/sedi_stun

Протестировано на arch linux. Для остальных дистрибутивов пункты 3, 4, 5 и 6 могут быть другими. 
