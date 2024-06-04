use std::{
    fs,
    io::{prelude::*, BufReader},
    net::{TcpListener, TcpStream},
};
// unwrap causes the program to panic if an error occurs
fn main() {
    let listener = TcpListener::bind("127.0.0.1:7878");
    let listener = match listener {
        Ok(l) => l,
        Err(e) => {
            println!("Error: {}", e);
            return;
        }
    };
    for stream in listener.incoming() {
        let stream = stream.unwrap();
        handle_connection(stream);
    }
}
fn handle_connection(mut stream: TcpStream) {
    let buf_reader = BufReader::new(&mut stream);
    let request_line = buf_reader.lines().next().unwrap().unwrap();

    let (status_line, filename) = if request_line == "GET / HTTP/1.1" {
        ("HTTP/1.1 200 OK", "hello.html")
    } else {
        ("HTTP/1.1 404 NOT FOUND", "404.html")
    };

    if let Some(contents) = handel_internal_server_error(fs::read_to_string(filename), &mut stream) {
        println!("File read successfully.");
        let length = contents.len();
        let response = format!("{status_line}\r\nContent-Length: {length}\r\n\r\n{contents}");
        stream.write_all(response.as_bytes()).unwrap();
    } else {
        println!("Error handled.")
    }
}

fn handel_internal_server_error<T, E>(input: Result<T, E>, stream: &mut TcpStream) -> Option<T>
where
    E: std::fmt::Debug,
{
    match input {
        Ok(value) => Some(value),
        Err(e) => {
            println!("Error: {:?}", e);
            stream.write_all("HTTP/1.1 500 INTERNAL SERVER ERROR\r\n\r\n".as_bytes()).unwrap();
            None
        }
    }
}