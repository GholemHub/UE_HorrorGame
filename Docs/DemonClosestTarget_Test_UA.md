# Демон: постійний вибір найближчого гравця

## Чому старий Blueprint не перемикав ціль

У `STT_AttackBabaj` змінна `DistanceToClosestCharacter` порівнюється з дистанціями на
наступних StateTree ticks, але не скидається перед новим `ForEach`. Таким чином вона
зберігає історичний мінімум, а не мінімальну дистанцію поточного кадру. Масив `Players`
також заповнюється лише після `ReceiveLatentEnterState`.

## Новий Blueprint-граф

Після C++ компіляції знайдіть вузол `Find Closest Player Target`.

1. У `Receive Latent Tick` викликайте `Find Closest Player Target`.
2. `World Context Object` — `Self`.
3. `Hunter` — ваш `AICharacter` (демон).
4. `Return Value` під'єднайте до `Branch`.
5. Із `True` запишіть `Out Target` у `ClosestCharacter`.
6. Після запису викличте `Move To Actor`, де `Goal = ClosestCharacter`.
7. Якщо демон не повинен бачити гравця у повністю закритій шафі, увімкніть
   `Ignore Safe Players`.

Старі вузли `GetAllActorsOfClass`, `Players`, `ForEachLoop`, `GetDistanceTo` та
`DistanceToClosestCharacter` після цього не потрібні.

Для продуктивності можна виконувати пошук раз на `0.1–0.25` секунди через накопичувач
часу замість кожного кадру. Але навіть при виклику кожен tick функція проходить лише
`GameState.PlayerArray`, а не всі actors світу.

## Тест

1. Запустіть PIE із двома гравцями та серверним вікном демона.
2. Поставте Player 1 ближче: `ClosestCharacter` має стати Player 1.
3. Підведіть Player 2 ближче до демона, ніж Player 1.
4. На наступному оновленні `ClosestCharacter` має змінитися на Player 2, а новий
   `Move To Actor` автоматично скасує попередній move request.
5. Поміняйте гравців місцями декілька разів.
6. Увімкніть `Ignore Safe Players`, сховайте поточну ціль у шафі та закрийте обидві
   стулки. Демон має вибрати наступного найближчого небезпечного гравця.

Якщо ви залишаєте старий ручний цикл, перед **кожним** `ForEachLoop` обов'язково
встановлюйте `DistanceToClosestCharacter = MAX_FLOAT` і `ClosestCharacter = None`.
