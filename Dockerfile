FROM ubuntu:latest

RUN apt-get update
    && apt-get install -y build-essential
    && apt-get install git
    && apt-get install -y valgrind
    && apt-get install -y vim
    && git config --global user.name "Parsa Langari"
    && git config --global user.email "parsa.langari@mail.mcgill.ca"

WORKDIR /app
COPY . /app

CMD [ "/bin/bash" ]
# here's the command to build the image with a tag "a3"
# docker build -t a3 .
# here's the command to run the image with a tag "a3"
# docker run -it a3
