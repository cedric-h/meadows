OUTDIR = pub
SOURCE = main.c geo.c gameplay.c math.c
CC = clang
COPY = index.html font.png
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

clean:
	rm -rf $(OUTDIR)

deps:
	sudo pacman -S clang lld nodejs npm
	npm install express

build:
	mkdir -p $(OUTDIR)
	$(CC) $(FLAGS) -o $(OUTDIR)/main.wasm $(SOURCE)
	cp $(COPY) $(OUTDIR)

all: build