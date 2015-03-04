
Item 4: 추론된 타입을 보는 법을 알아라.
===

1. 코드 편집 중에 보는 법: IDE 에디터
---
요즘 IDE들은 좋아서 변수 위에 마우스를 올리면 타입을 자동으로 띄워준다.

![](https://c2.staticflickr.com/4/3457/3868299211_55eda5cf1a.jpg)

다만 너무 복잡한 타입의 경우에는 `const
std::_Simple_types<std::_Wrap_alloc<std::_Vec_base_types<int,
std::allocator<int>>::_Alloc>::value_type>::value_type *`처럼 답이 안 나오는 상황이 벌어지므로 다음 방법들도 알아두자.<br/>
참고로 위 타입은 `const int *`이다.<br/>
직접 보고싶다면 다음 코드를 친 다음 `f` 위에 마우스를 올리면 된다. T에 해당하는 부분이 위의 내용으로 채워져 있을 것이다.
```C++
template<typename T>
void f(const T& param);

int main()
{
	const std::vector<int> vw;
	f(&vw[0]);
	return 0;
}
```

2. 컴파일 중에 보는 법: 컴파일러 진단
---
컴파일 에러를 내서, 에러 메시지를 통해 타입을 알아내는 방식이다.
```C++
// 에러 발생용 클래스. 일부러 선언만 하고 정의를 하지 않는다.
// TD == Type Displayer
template<typename T>
class TD;

const int* const x;
int f(const int&);

TD<decltype(x)> xType;
TD<decltype(f)> fType;
```
g++에서는 에러 메시지가 이렇게 나온다.
```
error: aggregate 'TD<const int* const> xType' has incomplete type and cannot be defined
error: aggregate 'TD<int(const int&)> fType' has incomplete type and cannot be defined                                                                                      
```


3. 런타임에 보는 법: Boost.TypeIndex 
---
런타임까지는 타입을 알 수 없지만, 출력 포맷을 마음대로 지정할 수 있다는 장점이 있다.

C++에서 기본적으로 지원해주는 방식은 `typeid` 키워드와 `std::type_info` 객체다.
```C++
struct MyStruct {};
class MyClass;

int num = 10;
const char* str = "hello";
MyStruct obj1;
MyClass* obj2 = nullptr;

std::cout << typeid(num).name() << std::endl;
std::cout << typeid(str).name() << std::endl;
std::cout << typeid(obj1).name() << std::endl;
std::cout << typeid(obj2).name() << std::endl;
```

g++에서는 이렇게 나온다.
```
i
PKc
8MyStruct
P7MyClass
```
...
참고로 i는 `int`, c는 `char`이라는 의미이고, P는 pointer, K는 konst(`const`)를 의미한다.<br/>
그리고 숫자는 구조체/클래스의 이름 길이다.

Visual Studio에서는 이렇게 나온다.
```
int
char const *
struct MyStruct
class MyClass *
```
훨씬 낫다.

하지만 이 방식에는 치명적인 문제가 있다.
```C++
template<typename T>
void f(const T& param)
{
  std::cout << typeid(T).name() << std::endl;
  std::cout << typeid(param).name() << std::endl;
}

int x;
f(x);
```

결과는 다음과 같다.
```
# g++
i
i
# VC++
int
int
```
* `typeid`를 쓰면 타입의 참조성과 `const`, `volatile` 모두 사라진다.
  * `typeid(consr int&)`처럼 타입 자체를 넣어주는 경우도 마찬가지.
  * `const char*`은 괜찮았는데!? 라고 생각할 수도 있겠지만 포인터가 가리키는 타입의 상수성은 포인터 자체의 상수성과 상관 없다는 것을 상기하자.

----------

해결법은 Boost 라이브러리에 있다. Boost.TypeIndex이다.
```C++
template<typename T>
void f(const T& param)
{
  std::cout
    << boost::typeindex::type_id_with_cvr<T>.pretty_name()
    << std::endl;
  std::cout
    << boost::typeindex::type_id_with_cvr<decltype(param)>.pretty_name()
    << std::endl;
}

int x;
f(x);
```
`type_id_with_cvr`이라는 이름에서 알 수 있듯이 `const`, `volatile`, reference를 모두 보존한다.
```
int
int const &
```