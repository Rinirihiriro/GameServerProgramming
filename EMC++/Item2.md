Item02: `auto` 타입 추론을 이해해라. (Understand `auto` type deduction.)
===
- `auto` 타입 추론은 템플릿 타입 추론과 완전히 동일하다.
  - 템플릿 타입 추론은 인수(Argument) `expr`을 가지고 타입 매개변수 `T`와 파라미터 타입 `ParamType`을 추론해낸다.
  - `auto` 타입 추론에서는 `auto`가 `T`의 역할을 하고, 타입 한정자들이 `ParamType`의 역할을 한다.
  - 단, 추론에 있어 한 가지 예외가 있다. (후술)

```C++
auto x = 27;        // T : auto, ParamType : auto
const auto cx = x;  // T : auto, ParamType : const auto
const auto& rx = x; // T : auto, ParamType : const auto&
```

### 예외
```C++
// 모두 int형 변수를 초기화 하는 문장.
int x1 = 27;
int x2(27);
int x3 = { 27 };
int x4{ 27 };

// 같은 문장에서 auto를 사용한 경우
auto y1 = 27;     // int
auto y2(27);      // int
auto y3 = { 27 }; // std::initializer_list<int>
auto y4{ 27 };    // std::initializer_list<int>
```
* 중괄호를 사용한 초기화를 사용하면, `auto`는 `std::initializer_list`로 추론된다.
  - 중괄호에 값이 하나밖에 없더라도, `std::initializer_list`가 만들어지고, 그 값 하나가 들어간다.
  - `auto`가 `std::initializer_list`로 추론되면 `std::initializer_list` 역시 템플릿이기에 템플릿 타입 추론이 한 번 더 일어난다.
    - 이 때는 중괄호 안의 모든 원소를 사용해서 추론한다. 하나라도 타입이 다른 원소가 있다면 추론에 실패한다.

```C++
auto x = { 1, 2, 3 };  // std::initializer_list<int>

template<typename T>
void f1(T param);

f1({1, 2, 3});         // Error.

template<typename T>
void f2(std::initializer_list<T> param);

f2({1, 2, 3});         // void f2<int>(std::initializer_list<int>)
```
* `std::initializer_list`로의 추론은 `auto`인 경우에만 적용된다. 템플릿 타입 추론에서는 먹히지 않는다!
  - 템플릿에서 사용할 시 `std::initializer_list`를 반드시 명시해주어야한다.

```C++
auto f()
{
    return { 1, 2, 3 };  // Error.
}

auto lambda = [](const auto& param){};
lambda({ 1, 2, 3 });     // Error
```
* C++14에서의 `auto` 반환형이나 람다의 `auto` 파라미터의 경우 `auto` 타입 추론이 아닌 템플릿 타입 추론을 사용한다.
  - 그래서 `std::initializer_list`로 추론이 이루어지지 않는다.
