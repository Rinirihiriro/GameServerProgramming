Item 3: `decltype`을 이해하라.
===
* `decltype`은 일반적으로 식의 타입을 **그대로** 돌려준다.
  * 일부 예외 상황이 있다. (후술)
```C++
int x;               // decltype(x) => int
const int cx;        // decltype(cx) => const int
int* px;             // decltype(px) => int*
int ax[10];          // decltype(ax) => int[10]
const int& crx = x;  // decltype(crx) => const int&
int f();             // decltype(f) => int()
```

----------

* `decltype`의 주 용도는 템플릿 함수의 반환형을 지정하는 것.
  * 특히 반환형이 템플릿 파라미터에 종속적인 경우.
  * C++11에서는 반환 타입 추적 문법(Trailing return type syntax)을 사용한다.
```C++
// C++11
template<typename Cont>
auto first(Cont& cont)
  -> decltype(cont[0])
{
  return cont[0];
}

std::vector<int> v = { 1, 2, 3 };
first(v);       // OK
first(v) = 10;  // OK
```
* `cont[0]`의 타입을 알 수 없으니 반환 타입을 알 수 없다.
  * 컨테이너에 무슨 타입이 들어있을지 알 수 없다.
  * 또한, 컨테이너의 종류마다 반환되는 타입이 일정하지 않다.
  * 해결을 위해 C++11에서는 반환 타입 추적을 추가한다.
    * `decltype`으로 어떤 타입이 반환될지를 결정한다.

----------

C++14는 auto 반환형을 쓰면 자동으로 반환타입을 추론하므로 반환 타입 추적이 필요 없다.
```C++
// C++14
template<typename Cont>
auto first(Cont& cont)
{
  return cont[0];
}

std::vector<int> v = { 1, 2, 3 };
first(v);       // OK
first(v) = 10;  // Error
```
* C++14에서는 자동으로 반환 타입을 추론한다.
    * C++11에서는 single-statement lambda의 경우에만 자동으로 추론해준다.
* 이 경우, 템플릿 타입 추론에 의해서 `first(v)`의 반환형이 `int`다!
  * 템플릿 타입 추론인 이유는 Item 2 참조.
  * Case 3인 값 전달에 해당된다.
  * `decltype(cont[0])` -> ... -> `int&` -> `int`
    * `int&` -> `int` 이 부분에서 참조가 사라진다.
  * 이 문제를 해결하려면 다른 문법이 필요하다.
``` C++
// C++14
template<typename Cont>
decltype(auto) first(Cont& cont)
{
  return cont[0];
}

// auto type deduction
const int& x = other;
auto myX1 = x;             // myX1 is int
decltype(auto) myX2 = x;  // myX2 is const int&
```
* `decltype(auto)`는 초기화 값을 가지고 `decltype` 타입 추론 과정을 거친다.
  * 즉, `first`는 `decltype(cont[0])`이 반환 타입이 된다.
  * 아래 `auto`를 사용하는 경우에도, `decltype(x)`가 `myX2`의 타입이 된다.

----------

> `first` 템플릿 함수는 rvalue를 받을 수 없는데, rvalue를 받을 수 있도록 온전하게 구현한 코드는 다음과 같다.

```C++
// C++11
template<typename Cont>
auto first(Cont&& cont)
  -> decltype(std::forward<Cont>(cont)[0])
{
  return std::forward<Cont>(cont)[0];
}

// C++14
template<typename Cont>
decltype(auto) first(Cont&& cont)
{
  return std::forward<Cont>(cont)[0];
}
```

### 예외
* `decltype`에 들어온 표현식의 결과가 lvalue라면 lvalue reference로 추론한다!
  * 단, 변수 이름 하나만 들어왔다면 원래 타입 그대로 추론한다.
```C++
int x = 0;
decltype(x) x1;    // int
decltype((x)) x2;  // int&
decltype(x,x) x3;  // int&
decltype(x=1) x4;  // int&
decltype(++x) x5;  // int&
```
* `decltype(x)`는 `x`가 lvalue지만 변수명만 넣었으므로 lvalue.
* `decltype((x))`는 `(x)`의 결과가 lvalue이므로 lvalue reference.
* `x,x`나 `x=1`등의 표현식들도 마찬가지로 결과가 lvalue이므로 lvalue reference.

이에 따른 문제상황:
```C++
// C++14
decltype(auto) inc1(int x)
{
  return x++;  // return int
}

decltype(auto) inc2(int x)
{
  return ++x;  // return int&
}
```
* 얼핏 보기에는 둘 다 문제 없는 코드같지만 inc2는 커다란 문제가 있다. - 지역변수의 참조를 반환한다!
  * `decltype(++x)`에서 `++x`는 lvalue이기 때문에 lvalue reference로 추론되어 반환형이 `int&`이다.

이런 예외상황은 실제로 자주 발생하지는 않는다...고 한다.