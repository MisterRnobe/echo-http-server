FROM gcc:latest

COPY main.c main.c
RUN gcc -o myapp main.c
CMD ["./myapp"]