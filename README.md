# Funduino - Arduino Multi-function Shield for school
Funduino is a easy use functions library for a cheap Arduino Multi-function Shield<br/>
![picture](https://s-media-cache-ak0.pinimg.com/originals/f7/1d/6a/f71d6a26fb99f469412439486c58a147.jpg)

## Functions
- 8 User controlled timers (repeat/once) up to 32 sec <code>timer_start(1,time,repeat=true);... if (is_timer(1))..</code>
- Button debounce logic - <code>if (button_pressed(1))...; button_released(#)</code> triggers.
- Button actual state <code>if (button(2))...</code>
- Display control functions including multiplexing all 4 segments <code>dispInt(value,[DEC|HEX|OCT|BIN]), display([int|float])</code>
- Direct led controls <code>led(1,true);led(2,false)</code>

## Extension for ArduBlock
My fork ardublock (https://github.com/tykefcz/ardublock) is extended for use these functions.
