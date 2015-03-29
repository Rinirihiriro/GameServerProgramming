\#0 TCP
===

[RFC793](http://tools.ietf.org/html/rfc793)
[RFC1122](http://tools.ietf.org/html/rfc1122#page-82)


[TOC]

Nagle Algorithm
---
### 개념
[RFC896](http://tools.ietf.org/html/rfc896)

TCP 통신에서, 패킷을 보낼 때 상대측의 ACK를 받은 뒤에 다음 패킷을 보내는 방식이다.
```sequence
Title: Without Nagle
A->B: SEQ "H"
A->B: SEQ "e"
A->B: SEQ "l"
A->B: SEQ "l"
A->B: SEQ "o"
```

```sequence
Title: With Nagle
A->B: SEQ "H"
B->A: ACK
A->B: SEQ "ello"
```

### 목적
Small packet problem을 해결해 TCP의 효율을 높이기 위해 고안되었다. 작은 크기의 패킷을 여러번 보내게 될 경우 수많은 헤더와 ACK 패킷으로 인해 대역폭을 낭비하게 되므로, 이들을 모아 큰 패킷을 적은 횟수 보내어 대역폭 효율을 높인 것이다.
대신 매 전송마다 ACK를 기다리게 되므로 Latency가 낮아진다는 단점이 있다.
그래서 높은 Latency가 요구되는 게임 서버 등에서는 보통 Nagle 알고리즘을 사용하지 않는다.

### ON/OFF 방법
Socket의 TCP_NODELAY 옵션을 지정한다. 
```C++
int opt_val = TRUE; // TRUE : Nagle Off, FALSE : Nagle On
setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &opt_val, sizeof(opt_val));
```

TCP의 단절 감지
---
### 유령 세션(실제로 끊겼지만 연결되어있는 세션)은 왜 생기는가?
![Tcp state diagram fixed new.svg](http://upload.wikimedia.org/wikipedia/commons/thumb/f/f6/Tcp_state_diagram_fixed_new.svg/1200px-Tcp_state_diagram_fixed_new.svg.png)
<br>By [Scil100](//en.wikipedia.org/wiki/en:Scil100). Licensed under [CC BY-SA 3.0](http://creativecommons.org/licenses/by-sa/3.0)

 A TCP connection may terminate in two ways: (1) the normal
            TCP close sequence using a FIN handshake, and (2) an "abort"
            in which one or more RST segments are sent and the
            connection state is immediately discarded.

TCP연결을 종료하는 방법, 즉 Established 상태에서 Closed 상태로 전이하는 방법은 두 가지이다.
            
1. 일반적인 Close 절차를 밟는다. (속칭 4-way handshaking 또는 FIN handshaking)
2. 상대쪽에서 보낸 RST segment를 받아 abort한다. (Half-Open connection discovery 등)

이 두 경우는 *실제로 통신이 가능한 상태*여야 적용될 수 있다.
랜선을 뽑는 등의 비정상적인 단절이 일어날 경우 통신 자체가 불가능하므로 결국 Established 상태로 남게 된다.
즉, 실제로는 통신이 불가능하지만 Established 상태로 남아있어 TCP 연결이 되어있는 것으로 간주되는 것이고, 이것이 곧 유령 세션이 되는 것이다.

### 단절을 감지하고 처리하는 방법은?
#### Keep-alive
일정 시간동안 데이터나 ACK 패킷이 오지 않으면 keep-alive 패킷을 보내 연결이 계속 이루어지고 있는지 확인하는 방법이다. default로는 꺼져있고, `SO_KEEPALIVE` 옵션을 지정해서 켜주어야 한다. default keep-alive timeout은 2시간(!)이다. 이 시간을 바꾸려면 레지스트리를 수정해주어야한다.(!!)
다행히 Windows 2000부터는 `SIO_KEEPALIVE_VALS` 옵션을 이용해서 수치를 변경할 수 있다.

#### Heartbeat


Linger Option
---
### 왜 필요한가? 어떤 경우에 쓰는가?
통신이 일어나는 중에 socket을 무작정 닫게 되면(Hard close) 아직 도달하지 못한 (보내지 못했거나 ACK를 받지 못한) 데이터들은 손실된다.
이런 문제가 없도록 socket을 close할 때 default설정은 graceful close로 되어있다. socket을 close해도 background에서 나머지 데이터를 전송한 뒤에 socket을 닫는다. 대신 사용자는 이 작업이 언제 종료되는지를 알 수 없다.
linger 옵션은 이 둘을 절충한 방식으로, socket을 닫지 않고 남은 데이터를 보내는 작업을 수행하다가, 작업이 일정 시간 이상 걸리는 경우 hard close하는 방법이다.
linger 옵션을 지정하고 closesocket를 호출하면 지정한 시간동안 함수는 block된다.

### 사용법
SO_LINGER 옵션을 설정한다. `linger` 구조체를 사용한다.
```C++
linger linger_opt = { 0, };  // linger struct
linger_opt.l_onoff = 1;      // linger on (non-zero value)
linger_opt.l_linger = 1;     // 1초동안 linger
setsockopt(sock, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));
```
`l_onoff`의 값을 0으로 줄 경우 default 설정인 graceful close가 된다.
`l_linger`의 값을 0으로 줄 경우 Hard closing이 된다.

> man page에서는 `close`나 `shutdown` 모두 block된다고 나와있지만 Windows에서는 `closesocket`을 호출할 때만 적용되고, `shutdown`을 호출할 때는 적용되지 않는다.

Graceful Shutdown
---
graceful close(또는 graceful shutdown, graceful disconnect)는 연결을 해제하기 전에 전송이 완료되지 않은 데이터를 모두 전송한 뒤 연결을 해제하는 방식을 말한다.
이와 반대로 전송되지 않는 데이터를 마저 전송하지 않고 (버리고) 연결을 해제하는 경우는 hard close 또는 abortive close라고 한다.

### 우아한 연결 종료는 어떻게 하면 될까?
위에서 socket을 닫을 때 default 방식이 graceful close라고 했고, graceful close와 hard close를 반반 섞은 linger option도 소개했다. 하지만 두 방식 모두 *완벽하게* graceful close를 하는 것은 아니다. half-duplex close sequence에서는 의도치않게 abort가 발생할 수 있기 때문이다.

기본적으로 establish된 TCP connection을 close 하면 상대에게 FIN segment를 전송한다. 하지만, kernel에 recv 데이터가 남아있는 상태에서 connection을 close하거나, close 한 이후에 데이터가 들어오면 전송된 데이터가 손실된다는 의미로 RST segment를 전송한다.
상대측에서는 이 RST segment가 connection reset를 의미하는지 data loss를 의미하는지 구분할 수가 없기 때문에 단순히 전자로 해석해 abort하게되고, 결국 hard close된다.

이러한 상황을 막기 위해서는 *모든 데이터를 받은 뒤에* socket을 닫아야 한다. FIN segment는 "더이상 보낼 데이터가 없다"는 의미이므로 FIN segment까지 받은 뒤에 socket을 닫으면 될 것이다. 이를 위해 socket을 닫지 않고 FIN segment를 보낼 방법이 필요한데, `shutdown`이나 `WSASendDisconnect`를 사용하면 socket을 완전히 닫지 않고도 FIN segment를 전송할 수 있다.

```C++
// 더이상 보낼 데이터가 없으면
shutdown(sock, SD_SEND);  // FIN segment를 보낸다. 더이상 send는 할수 없다.
// FIN segment까지 받는다.
while ((result = recv(sock, buf, BUF_SIZE, 0)) > 0);
// recv도중 Error가 발생하면 graceful close는 불가능. abort한다.
if (result == SOCKET_ERROR)
  ...
// 모두 recv 했으므로 socket을 닫는다.
closesocket(sock);
```

`WSASendDisconnect`는 `shutdown`에 `SD_SEND`를 지정해준 것과 같지만 추가로 disconnect data를 보낼 수 있다. (기존의 TCP에서는 지원하지 않는 WinSock만의 기능이다.)