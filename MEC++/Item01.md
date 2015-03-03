Item 1: 템플릿 타입 추론을 이해하라. (Understand template type deduction.)
===

- 템플릿 타입 추론은 C++11의 `auto` 타입 추론의 기초가 된다.
- 일부 경우, `auto`의 타입 추론은 템플릿 타입 추론보다 덜 직관적이다.
  - (`auto` 문맥에 템플릿 타입 추론을 적용했을 때, 템플릿에 적용했을 때 보다 가끔 덜 직관적이다.)

함수 템플릿은 다음처럼 생각할 수 있다.
```C++
template<typename T>
void f(ParamType param);
```

그리고 이 함수를 `f(expr)`처럼 부를 수 있다.
컴파일러는 `expr`을 사용해 `T`와 `ParamType`을 추론한다.
여기서 `ParamType`은 `const`나 참조 한정자 등을 쓸 수 있기 때문에 많은 경우에 추론된 두 타입의 결과가 다르다.

예컨대 이렇게 함수 템플릿이 정의되어있고,
```C++
template<typename T>
void f(const T& param);
```
이렇게 호출하면,
```C++
int x = 0;
f(x);
```
`T`는 `int`로 추론되고, `ParamType`은 `const int&`로 추론된다.<br/>
`T`의 타입은 `expr`과 `ParamType`을 토대로 추론된다.

다음의 세 가지 경우가 있다.

- `ParamType`이 포인터이거나 참조형이지만 universal reference가 아닌 경우.
- `ParamType`이 universal refernce인 경우.
- `ParamType`이 포인터나 참조형이 아닌 경우.

### 1. ParamType이 포인터이거나 참조형이지만 universal reference가 아닌 경우.
1. `expr`이 참조형이라면, 참조 부분을 무시한다.
2. `T`를 결정하기 위해 `expr`의 타입을 `ParamType`에 패턴매칭한다.

```C++
template<typename T>
void f(T& param);

int x = 27;
const int cx = x;
const int& rx = x;

f(x);           // void f<int>(int&)
f(cx);          // void f<const int>(const int&)
f(rx);          // void f<const int>(const int&)
```
* 참조형 파라미터에 상수 객체를 넘기면 상수성이 보존된다.
* 인수(argument)의 참조성은 무시된다.

```C++
template<typename T>
void f(const T& param);

int x = 27;
const int cx = x;
const int& rx = x;

f(x);           // void f<int>(const int&)
f(cx);          // void f<int>(const int&)
f(rx);          // void f<int>(const int&)
```

```C++
template<typename T>
void f(T* param);

int x = 27;
const int* px = &x;

f(&x);          // void f<int>(int*)
f(px);          // void f<const int>(const int*)
```

### 2. ParamType이 universal refernce인 경우.
- expr이 lvalue인 경우, T와 ParamType 모두 lvalue reference로 추론된다.
- expr이 rvalue인 경우, 1번 룰을 따른다.

```C++
template<typename T>
void f(T&& param);

int x = 27;
const int cx = x;
const int& rx = x;

f(x);           // void f<int&>(int&)
f(cx);          // void f<cosnt int&>(const int&)
f(rx);          // void f<const int&>(const int&)
f(27);          // void f<int>(int&&)
```
* Universal reference는 lvalue와 rvalue를 구분한다.

### 3. ParamType이 포인터나 참조형이 아닌 경우.
즉, `ParamType`이 값 전달일 경우이다.

1. `expr`이 참조형이라면, 참조 부분을 무시한다.
2. `expr`이 `const`이거나 `volatile`이라면, `const`와 `volatile`을 무시한다.

```C++
template<typename T>
void f(T param);

int x = 27;
const int cx = x;
const int& rx = x;

f(x);           // void f<int>(int)
f(cx);          // void f<int>(int)
f(rx);          // void f<int>(int)
```
* 파라미터가 값 전달일 경우 const와 volatile은 보존되지 않는다.
  - 원본이 const라고 복사본이 const일 이유는 없으니까...
  - 어차피 원본은 보존되고...

----------

```C++
template<typename T>
void f(T param);

const char* const ptr = "String";

f(ptr);         // void f<const char*>(const char*)
```
* 상수 객체의 포인터의 경우 상수성이 유지된다!
  - 포인터 자체의 상수성은 상관 없다.
    - `const char* const` -> `const char*`
  - 포인터가 가르키는 객체의 상수성을 지우게 되면 원본 객체가 수정될 수 있다.
  - 그래서 포인터가 가르키는 객체의 상수성은 유지된다.

### 인수(argument)에 배열이 오는 경우
* 배열 타입은 포인터 타입과 다르다!
  - 배열 타입을 포인터 타입으로 변질(decay)시킬 수 있는 것 뿐이다.

```C++
template<typename T>
void f(T param);

const char arr[] = "String";

f(arr);         // void f<const char*>(const char*)
```
* 값 전달의 경우 배열을 파라미터로 받는 방법이 없기 때문에 포인터로 decay된다.

----------

```C++
template<typename T>
void f(T& param);

const char arr[] = "String";

f(arr);         // void f<const char[7]>(const char (&)[7])
```
* 참조형의 경우 배열의 참조형이 존재하기 때문에 배열의 참조형으로 받는다.

----------

```C++
template<typename T, std::size_t N>
std::size_t arraySize(T (&)[N])
{
    return N;
}

const char arr[] = "String";

arraySize(arr); // std::size_t f<const char, 7>(const char (&)[7])
                // => 7
```
* 비타입 매개변수를 이용해서 배열의 길이를 받아올 수도 있다!

### 인수(argument)에 함수가 오는 경우
* 함수 타입 역시 존재하며, 배열처럼 decay된다.

```C++
template<typename T>
void f1(T param);

template<typename T>
void f2(T& param);

void someFunc(int, double);

f1(someFunc);   // void f1<void(*)(int, double)>(void(*)(int, double))
f2(somefunc);   // void f1<void(int, double)>(void(&)(int, double))
```