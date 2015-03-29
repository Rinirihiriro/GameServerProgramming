Ex Item 1: Generic programming: Type list
===
type list를 만들면서 generic programming을 배워보자.

original article: [Generic Programming:Typelists and Applications](http://www.drdobbs.com/generic-programmingtypelists-and-applica/184403813)

[TOC]

## Type list란?
이름 그대로 type을 담는 list를 type list라고 한다.
C++에서는 타입을 데이터처럼 list에 넣어둔 다는 것이 불가능해보이지만, template을 사용하면 얼마든지 가능하다.

```C++
template <class H, class T>
struct typelist
{
  typedef H head;
  typedef T tail;
};

// C++11
template <class H, class T>
struct typelist
{
  using head = H;
  using tail = T;
};
```
H에는 head라는 별명을 주고, T에는 tail이라는 별명을 주었다. 이렇게 하면 `typelist<int, char>::head`처럼 H와 T 타입을 가져올 수 있다.

이 정의만으로 type list는 완성이다. 실제 사용은 이렇게 한다.

```C++
typedef typelist<float, typelist<double, long double> >
  floating_point_types;

// C++11
using floating_point_types =
  typelist<float, typelist<double, long double>>;
```
`typelist`도 타입이기 때문에 `typelist`의 template argument로 사용될 수 있다.

list의 끝을 명확히 알 수 있도록 끝을 나타내는 타입을 추가하자.

```C++
struct null_typelist {};

typedef typelist<float, typelist<double,
  typelist<long double, null_typelist> > >
  floating_point_types;

// C++11
using floating_point_types =
  typelist<float, typelist<double,
  typelist<long double, null_typelist>>>
```

## `typelist` 생성을 선형화 하는 세 가지 방법 +a
`typelist`를 만든 것까지는 좋았는데 타이핑이 고통스럽다. 들어가는 원소가 많아지면 정말 볼 만해질 것이다. 게다가 오래된 C++ 파서가 `>>`를 비트 연산자로 인식하는 것을 막기 위해 사이사이 공백을 넣는 것을 까먹어서도 안 된다.

이 상황을 타개하기 위해 매크로를 사용해보자.

```C++
#define TYPELIST_1(T1) typelist<T1, null_typelist>
#define TYPELIST_2(T1, T2) typelist<T1, TYPELIST_1(T2) >
#define TYPELIST_3(T1, T2, T3) typelist<T1, TYPELIST_2(T2, T3) >
...
#define TYPELIST_50...
```
각 매크로 함수는 이전 매크로 함수를 사용해서 정의된다.
... 이건 뭔가 아니다 싶은 느낌이 팍팍 느껴진다. N개의 원소를 가진 list를 만들기 위해서는 미리 N개의 매크로를 만들어놓아야 하고, 만들어 둔 매크로의 한계보다 많은 원소를 넣으려면 추가로 매크로를 만들던가 처음처럼 깡으로 해결해야한다.

게다가 우리가 상상도 못한 곳에서 문제가 일어진다.

```C++
typedef TYPELIST_2(vector<int, allocator>, vector<int, my_allocator>)
  storage_choices;
```
놀랍게도, TYPELIST_2에는 **4개의 인수가 넘겨진다.**

`vector<int`, `allocator>`, `vector<int`, `my_allocator>`

충격과 공포의 도가니다. 구시대의 유물 매크로는 잊어버리고 다른 방법을 찾는 것이 정신 건강에 이로울 것 같다.

---

템플릿은 매크로보다 좋은 발상이다.
```C++
template <class T1>
struct cons<T1, null_typelist, null_typelist>
{
  typedef typelist<T1, null_typelist> type;
};

template <class T1, class T2>
struct cons<T1, T2, null_typelist>
{
  typedef typelist<T1, typelist<T2, null_typelist> > type;
};

template <class T1, class T2 = null_typelist, class T3 = null_typelist>
struct cons
{
  typedef typelist<T1, typelist<T2, typelist<T3, null_typelist> > > type;
};
```
```C++
typedef cons<float, double, long double>::type floating_point_types;
```
순서가 거꾸로라서 눈치채지 못했겠지만 템플릿 부분 특수화(template partial specialization)를 사용한 기법이다. 물론 이 순서대로 쓰면 컴파일 에러가 나므로 특수화 하는 코드 위에 기본 `cons` 템플릿 클래스(여기서는 구조체)를 선언해주어야한다.

실제로 `cons<int, char>`를 쓰면 `struct cons<T1, T2, T3>`의 특수화형인 `struct cons<T1, T2, null_typelist>`를 사용하게 된다. 그 과정을 따라가보자.

1. 컴파일러는 `cons<int, char>`를 보고 `cons` 템플릿 클래스(여기서는 구조체)를 본다.
2. `cons`가 3개의 template parameter를 가지고 있는 것을 보고선, `cons<int, char>`은 2개의 parameter를 가지고 있으니, `cons`의 세 번째 parameter가 default argument를 갖는지 확인한다.
3. default argument가 `null_typelist`이므로 `cons<int, char>`은 `cons<int, char, null_typelist>`가 된다. 이 parameter들로 가장 specialize된 `cons`를 찾는다.
4. `cons<T1, T2, null_typelist>`를 찾아 `struct cons<int, char, null_typelist>` 구조체를 만든다.

크게 어려울 것 없다.

그런데 여기서 만약 `cons<int>`을 썼다면 어떻게 되었을까? `cons<int, null_typelist, null_typelist>`로 변한 것 까지는 따라갈 수 있을 것이다. 하지만 이 parameter들로 매칭되는 특수화형은 `cons<T1, null_typelist, null_typelist>`와 `cons<T1, T2, null_typelist>` 두 가지이다!

위에서 3번 과정을 다시 보자. *가장 specialize된 `cons`를 찾는다*고 했다. `cons<T1, null_typelist, null_typelist>`와 `cons<T1, T2, null_typelist>`중 `cons<int, null_typelist, null_typelist>`에 더 specialize된 것은 무엇일까? 누가 보아도 `cons<T1, null_typelist, null_typelist>`이다. 컴파일러도 적절히 추론해서 `cons<T1, null_typelist, null_typelist>`를 선택한다.

문법적으로 아무 문제 없이 사용할 수 있다. 하지만 확장성이 문제다. argument를 4개 이상 넣으려면 parameter를 전부 뜯어고쳐야하고 누가 봐도 노가다가 상당해보인다. 좀 더 편한 방법이 없을까?

> 원문에서는 `cons`에 default parameter를 지정하지 않아서 그대로 사용하면 컴파일 에러가 난다. default parameter를 꼭 추가해주자.

---

세 번째 방법은 function signatures를 쓰는 방법이다.

```C++
template <class Func>
struct cons;

template <class T1>
struct cons<void(T1)>
{
  typedef typelist<T1, null_typelist> type;
};

template <class T1, class T2>
struct cons<void(T1, T2)>
{
  typedef typelist<T1, typelist<T2, null_typelist> > type;
};

template <class T1, class T2, class T3>
struct cons< void(T1, T2, T3)>
{
  typedef typelist<T1, typelist<T2, typelist<T3, null_typelist> > > type;
};
```
```C++
#define TYPELIST(a) cons< void a >::type

typedef TYPELIST((float, double, long double)) floating_point_types;
```
이번에도 템플릿 특수화를 썼다. Func를 특정 function signature로 특수화한 것이다. `struct cons`를 선언만 하고 정의하지 않았기 때문에 특수화되지 않은 `cons`를 인스턴스화 하려 하면 에러를 낼 것이다.

선언 이후에는 인수가 1개, 2개, 3개인 function signature를 template argument로 받는 템플릿 특수화들을 만들어낸다. 이렇게 하면 N개의 원소를 받는 `cons`를 만들기 위해 기존 코드를 만질 필요가 없어진다.

실제 사용 예가 `TYPELIST((int, char))` 이런 식인데, 매크로 덕분에 어색해보인다. 전처리기를 통과하면 `cons< void (int, char) >::type`으로 바뀌고, 특수화된 `struct cons<void(T1, T2)>`를 사용하게 된다.

> 원문에서는 함수 포인터를 사용했는데, 어차피 실제 instance가 사용되는 것도 아니고 template argument로만 사용되기 때문에 굳이 함수 포인터일 필요는 없다.

---

***이 부분은 원문에 없던 내용이다.***

실제로는 위 방법도 마찬가지로 번거롭다. N개 argument를 받는 템플릿 특수화를 일일이 구현해 주어야 한다는 것은 아무리 생각해도 이상하다. 임의의 argument들을 받아 `typelist`를 만드는 방법은 없을까?

Variadic template(Parameter pack)을 사용하면 된다.

```C++
template <class Head, class ... Tail>
struct cons
{
    using type = typelist<Head, typename cons<Tail ...>::type>;
};

template <class Head>
struct cons<Head>
{
    using type = typelist<Head, null_typelist>;
};
```
```C++
typedef cons<float, double, long double>::type floating_point_types;
```

이것이 전부다. 깔끔하지 않은가?

원리는 간단하다. 예컨대 `<int, char, short, float>`을 `<class Head, class ... Tail>`에 매칭하면 `Head` == `int`, `Tail` == `char, short, float`이 된다. 그럼 `type`은 `typelist<Head, typename cons<Tail ...>::type>`이므로 `typelist<int, cons<char, short, float>::type>`이 된다. 재귀적으로 `cons<char, short, float>`, `cons<short, float>`까지 만들어진다.
마지막에는 `cons<float>`가 남는데, 이것은 특수화된 `cons<Head>`를 이용하게 되어 이때의 `type`은 `typelist<float, null_typelist>`이 된다. 순서대로 보면 이렇다.

1. `cons<int, char, short, float>::type`
2. `typelist<int, cons<char, short, float>::type>`
3. `typelist<int, typelist<char, cons<short, float>::type>>`
4. `typelist<int, typelist<char, typelist<short, cons<float>::type>>>`
5. `typelist<int, typelist<char, typelist<short, typelist<float, null_typelist>>>>`

어떻게 바뀌는지 보이는가?

---

앞으로 `typelist`를 만드는 코드는 `cons<...>::type`으로 표기할 것이다. 2번째 방법 (또는 마지막에 제시한 방법)을 선택한 것이다.

## Ad-hoc Visitation
Visitor 패턴은 기존의 클래스 구조에 손을 대지 않고 새로운 기능을 추가하는 방법을 제안한다. 조금 어렵게 설명하면 알고리즘과 클래스 구조를 분리한다.