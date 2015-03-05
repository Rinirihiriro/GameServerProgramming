Item 6: `auto`가 예상대로 추론되지 않는다면, '명시적 타입 초기화' 관용구를 써라.
===
`auto`를 쓰면 예상치 못한 타입이 튀어나올 가능성이 있다.
```C++
std::vector<bool> f();

// swap
auto tmp = v[0];
v[0] = v[1];
v[1] = tmp;

std::cout << (v[0] ? "T" : "F") << (v[1] ? "T" : "F") << std::endl;
```
`FT`가 출력될 줄 알았는데 실제 결과는 `FF`다.<br/>
`tmp`의 타입을 찍어보면, `std::vector<bool>::reference`나 `std::_Bit_reference`로 나온다. 이것은 `std::vector<bool>`전용 래퍼런스 프록시 객체다. 즉, `tmp`는 `v[0]`를 '복사'한 것이 아니라 '참조'한 것이다.

> `std::vector<bool>`이 전용 래퍼런스 프록시 객체를 가지는 이유는 내부적으로 `bool`형 데이터를 바이트가 아닌 비트 단위로 관리하기 때문이다.
> 비트 단위로는 주소를 가져올 수 없기 때문에 일반적인 래퍼런스나 포인터로는 vector 안의 특정 원소를 참조할 수 없다. 그래서 래퍼런스를 대신하는 프록시 객체를 만들어 돌려주는 것이다.

### 명시적 타입 초기화 관용구 (The explicitly typed initializer idiom)
이 이름은 저자인 Scott Meyers가 지었다.

생김새는 별 거 없고 `auto`를 초기화하는 값에 `static_cast`만 해준 것이다.
```C++
auto variable = static_cast<Type>(initialize_value);
```

위 예제의 경우 아래처럼 바꾸면 정상 동작한다.
```C++
std::vector<bool> v = { true, false };

// swap
auto tmp = static_cast<bool>(v[0]);
v[0] = v[1];
v[1] = tmp;

std::cout << (v[0] ? "T" : "F") << (v[1] ? "T" : "F") << std::endl;
```
