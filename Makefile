OUT = pub/build/main.wasm
SOURCE = main.c geo.c gameplay.c math.c
CC = clang
FLAGS =   --target=wasm32 \
          -O3 \
          -g \
          -fvisibility=default \
          -flto \
          -nostdlib \
          -Wl,--export-all \
          -Wl,--no-entry \
          -Wl,--lto-O3 \
          -Wl,--allow-undefined \
          -Wall \
          -mbulk-memory

all:
	$(CC) $(FLAGS) -o $(OUT) $(SOURCE)

clean:
	rm -rf $(OUT)