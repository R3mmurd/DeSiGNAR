<em>Read this in other languages: <a href="MODERNIZATION.md">English</a></em>

# Posibles modernizaciones con estándares estables de C++

DeSiGNAR actualmente apunta a C++17. Este es un relevamiento de lo que
adoptar un estándar más nuevo *y estable* podría aportarle a la
biblioteca — primero C++20 (ya disponible en GCC, Clang y MSVC desde hace
varios años, genuinamente estable), y luego una sección más breve sobre
C++23 (más nuevo, el soporte de los compiladores todavía se está poniendo
al día, pero vale la pena conocerlo). Nada de esto está implementado — es
un menú de opciones para elegir, cada una con una nota aproximada de
costo/beneficio.

## C++20

### Concepts, en lugar de requisitos implícitos de plantillas

Hoy, un parámetro de tipo como `Cmp`, `Distance`, o el parámetro de tipo de
grafo `GT` usado en todo `graphalgorithms.hpp` está restringido solo por
convención, y un error de compilación aparece recién en las profundidades
de una instanciación de plantilla si te equivocás. Un `concept` convierte
eso en una única declaración legible y verificada:

```cpp
template <class GT>
concept GraphType = requires(GT g, Node<GT>* n) {
  { g.get_num_nodes() } -> std::same_as<nat_t>;
  { g.for_each_node([](Node<GT>*) {}) };
  // ...
};

template <GraphType GT, class Distance = DefaultDistance<GT>>
class Dijkstra { ... };
```

Pasar el tipo equivocado ahora falla en el punto de la llamada con "GT no
satisface GraphType: falta get_num_nodes()" en lugar de una página de
errores anidados de instanciación de plantillas desde el interior de
`Dijkstra`. Este es también el punto más *didáctico* de esta lista — los
concepts hacen explícito y legible el contrato implícito que cada
plantilla de esta biblioteca ya tiene, lo cual encaja especialmente bien
en una biblioteca con fines de enseñanza.

### `operator<=>` para el código de comparación de iteradores/nodos

`RandomAccessIterator` (iterator.hpp) escribe a mano `<`, `<=`, `>`, `>=`
a partir de una única comparación de `get_location()`; varias
comparaciones de claves en árboles/heaps hacen lo mismo. La comparación de
tres vías (three-way comparison) de C++20 colapsa ese código repetitivo:

```cpp
auto operator<=>(const Iterator& it) const
{
  return const_me().get_location() <=> it.get_location();
}
```

Una sola función genera las cuatro (más `==`/`!=`) en lugar de cuatro
escritas a mano que hay que mantener sincronizadas.

### Ranges

`for_each_node()`/`for_each_arc()` (graph.hpp) y las distintas funciones
libres `for_each_it()` (italgorithms.hpp) son anteriores a `<ranges>` y
usan un estilo de paso de callbacks en lugar de eso. Una vez que los
iteradores de cada contenedor satisfagan `std::ranges::forward_range` (ya
lo hacen en su mayoría, gracias al trabajo hecho para que sean compatibles
con los algoritmos de `std::`), los contenedores pasan a poder usarse
directamente con los adaptadores de rangos:

```cpp
for (auto& node : graph.nodes() | std::views::filter(is_active))
  ...
```

Esto es aditivo, no un reemplazo — los métodos `for_each_*` basados en
callbacks existentes se mantendrían (son los que permiten un recorrido
tipo DFS/BFS con salida anticipada, algo que un range simple no puede
expresar tan naturalmente), pero exponer una vista compatible con
`std::ranges` junto a ellos permitiría que los contenedores de esta
biblioteca se compongan directamente con la cadena de adaptadores de
rangos estándar, sin que quien la use tenga que escribir a mano adaptadores
`std::begin(x)`/`std::end(x)`.

### `std::jthread` + `std::stop_token` para los contenedores de concurrencia

`ThreadPool` (threadpool.hpp) actualmente se apaga mediante una "píldora
envenenada" manual — se envía una tarea centinela por cada worker para que
cada uno se despierte, reconozca el `std::function` vacío, y termine; luego
el destructor hace `join()` de cada `std::thread` a mano. `std::jthread`
fue diseñado exactamente para esto: se une (join) automáticamente en su
destructor y lleva un `std::stop_token` que una tarea en ejecución puede
consultar para saber que debe terminar cooperativamente, sin necesitar el
truco de la tarea centinela:

```cpp
std::vector<std::jthread> workers;
// ...
workers.emplace_back([this](std::stop_token stoken)
{
  while (!stoken.stop_requested())
  {
    // para aprovechar esto del todo, ConcurrentQueue::get() también
    // tendría que volverse interrumpible ante una solicitud de stop
  }
});
```

La única complicación: `ConcurrentQueue::get()` hoy bloquea
incondicionalmente, así que aprovechar todo el beneficio también implica
enseñarle a despertarse ante una solicitud de stop (por ejemplo,
`condition_variable_any::wait(lock, stoken, pred)`), no solo cambiar
`std::thread` por `std::jthread`.

### `std::span` para acceso masivo de solo lectura

Varios lugares (por ejemplo, los constructores masivos de
`FixedArray`/`DynArray`, o un hipotético caso de uso "dame una vista
contigua del almacenamiento de este arreglo sin copiar") hoy o copian a un
`DynArray` o requieren que quien llama ya tenga uno. `std::span<const T>`
es la vista estándar, no propietaria, de un rango contiguo, y permitiría
que por ejemplo `DynArray(std::span<const T>)` acepte un arreglo C plano,
un `std::vector`, u otro `DynArray` sin una copia intermedia ni depender
únicamente de un constructor con initializer-list.

### `constexpr`/`consteval` para los helpers numéricos puros

`mix64()` (bloomfilter.hpp), los helpers de combinación de hash en
hash.hpp, y los helpers de comparación numérica de math.hpp son todas
funciones puras sin E/S ni asignación de memoria — buenas candidatas para
`constexpr`, permitiendo que quien ya conoce sus entradas en tiempo de
compilación (por ejemplo, un `BloomFilter` de tamaño fijo calculado a
partir de una constante `expected_items` conocida en tiempo de
compilación) obtenga ese cálculo resuelto en tiempo de compilación en
lugar de en cada ejecución del programa.

### `std::format` para los helpers de E/S de grafos

`OutputGraph`/`DotGraph` (graphutilities.hpp) arman su salida encadenando
`operator<<` sobre `std::ostream`. `std::format` (y `std::print` una vez
que esté ampliamente disponible) se lee de forma más directa para la forma
de "plantilla de texto con marcadores de posición" que tiene la mayor
parte de esta salida — y, a diferencia del estado de los manipuladores de
`iostream` (`std::hex`, `std::setw`, etc.), no se filtra entre llamadas
que se olvidan de reiniciarlo.

## C++23 (más nuevo — verificá el soporte de tu toolchain antes de depender de esto)

### Deducing `this` — podría eliminar por completo el código repetitivo de `me()`/`const_me()` del CRTP

Este es el punto más importante para este código en particular. Cada
clase base de iterador (`BasicIterator`, `ForwardIterator`, ...) usa el
patrón CRTP clásico de "castear `this` hacia abajo al tipo derivado":

```cpp
protected:
  Iterator& me() { return *static_cast<Iterator*>(this); }
  const Iterator& const_me() const { return *static_cast<const Iterator*>(this); }
```

El parámetro de objeto explícito de C++23 ("deducing this") reemplaza todo
el idioma de clase-base-CRTP-más-cast con un parámetro de plantilla
ordinario en la propia función miembro:

```cpp
template <class Self>
auto&& get_curr(this Self&& self)
{
  return self.get_current();
}
```

Sin `static_cast`, sin un par separado de sobrecargas const/no-const, sin
un parámetro de plantilla CRTP encadenado a través de cada clase base de
la jerarquía — la misma fragilidad de la cadena CRTP de tres niveles que
este proyecto tuvo que arreglar a mano en `TArrayIterator` durante el
trabajo de compatibilidad con `std::` (agregando un parámetro `Derived`
explícito para que `operator+`/`operator-` devuelvan el tipo hoja
correcto) no habría sido posible cometerla en primer lugar, ya que no
existiría un tipo CRTP intermedio que devolver por error. Esto sería una
reescritura genuinamente invasiva de `iterator.hpp` y de cada clase
`*Node`/`*Iterator` construida sobre él — vale la pena hacerlo de forma
deliberada como su propio proyecto, no como un cambio incidental.

### `std::expected<T, E>` para la disyuntiva excepción-vs-código-de-error

Varios lugares de este código lanzan excepciones (`std::domain_error`,
`std::overflow_error`, `std::length_error`) para condiciones que quien
llama tal vez quiera verificar sin pagar el costo de manejar excepciones
ni escribir un `try`/`catch` (por ejemplo, `ClosestPair::compute()` con
menos de dos puntos, o el constructor de `BloomFilter` con tamaño cero).
`std::expected<T, E>` le da a esos puntos de llamada una forma de
devolver "un `T`, o una razón documentada de por qué no pudo producir
uno" sin que las excepciones sean la única opción — útil en particular
para las clases de algoritmos (los propios `FloydWarshall`, `EdmondsKarp`,
etc. de esta biblioteca) donde quien llama, en un cómputo de ruta
crítica, razonablemente podría preferir verificar en lugar de lanzar.

### `operator[]` multidimensional

`MultiDimArray::Slice::operator()` (array.hpp) existe específicamente
porque antes de C++23, `operator[]` solo podía tomar exactamente un
argumento — `operator()` era la solución alternativa para lograr un
acceso estilo `matrix[i, j]`. C++23 permite `operator[](i, j)`
directamente:

```cpp
T& operator[](nat_t i, nat_t j) { return data[i * cols + j]; }
```

Esto sería un reemplazo directo una vez que se eleve la versión mínima de
compilador soportada — `operator()` podría quedar como un alias
deprecado por compatibilidad, o eliminarse en un cambio de versión mayor.

### `std::flat_map`/`std::flat_set`

Contenedores asociativos respaldados por un vector contiguo y ordenado —
una compensación (trade-off) genuinamente distinta a la de cada estructura
asociativa que esta biblioteca ya tiene (tablas hash, y los seis tipos de
árbol balanceado), optimizada para cargas de trabajo de "construir una
vez, leer muchas veces" donde la localidad de caché le gana al recorrido
de punteros O(log n) de un árbol. Vale la pena mencionarlo tanto como
oportunidad *didáctica* como de implementación: es un buen ejemplo de
"misma interfaz, distinta compensación de big-O/caché" para poner junto a
las variantes de árbol/hash que ya existen en esta biblioteca.
