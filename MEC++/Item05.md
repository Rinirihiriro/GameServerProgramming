Item 5: 명시적 타입 선언보다는 `auto`를 써라.
===
### 변수 초기화
```C++
int a;        // 초기화되지 않은 변수는 위험하다!
auto b;       // Error. auto는 반드시 초기화를 해야한다.
auto c = 10;  // c는 int
```

### 복잡하고 장황한 타입
```C++
std::vector<int> v;
for (std::vector<int>::iterator it = v.begin(); it != v.end(); ++it);
for (auto it = v.begin(); it != v.end(); ++it);
```

### Closure 등 명시적으로 타입을 적기 힘든 경우
```C++
auto ret_1 = []{ return 1; };
auto power_of_2 = std::bind(pow, 2, std::placeholders::_1);
```
> `std::function`을 쓰는 것도 가능하지만, 함수의 시그니처를 직접 적어주어야 하고, 메모리나 속도면에서 추가적인 오버헤드가 발생한다.

### 유추하기 힘든 함수의 반환형
```C++
std::map<std::string, int> m;
for (const std::pair<std::string, int>& p : m);
// const std::pair<const std::string, int>&가 정확한 반환 타입이다.
// m의 원소가 복사되어 임시 객체가 만들어지고, 그 임시객체를 p가 참조하면서 문제가 생긴다.
for (const auto& p : m);
```

### `auto`가 실패하는 일부 경우
#### Item 2. 중괄호 초기화
```C++
int x = { 10 };
auto y = { 10 };  // std::initializer_list<int>
```

#### Item 6. 원하는 타입으로 추론되지 않는 경우 (보이지 않는 프록시 객체)
```C++
float x = 10.0;
auto y = 10.0;    // double

std::vector<bool> f();
auto z = f()[0];  // std::vector<bool>::reference
```
