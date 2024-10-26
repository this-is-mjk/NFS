use std::io;

fn number_of_rounds() -> usize {
    // Declare string
    let mut input = String::new();

    // Input string
    io::stdin().read_line(&mut input).unwrap();

    // Return number
    return input.trim().parse().unwrap();
}
fn take_point() -> Point {
    // Declare string
    let mut input = String::new();

    // Input string
    io::stdin().read_line(&mut input).unwrap();

    // Split string
    let mut iter = input.trim().split_whitespace();

    // Return point
    return Point {
        x: iter.next().unwrap().parse().unwrap(),
        y: iter.next().unwrap().parse().unwrap()
    };

}
fn distance(p1: &Point, p2: &Point) -> i32 {
    // Return distance
    (p1.x - p2.x).pow(2) + (p1.y - p2.y).pow(2)
}
struct Point {
    x: i32,
    y: i32
}
fn main() {
    // number of rounds
    let rounds = number_of_rounds();

    for _ in 0..rounds {
        let mut points = [Point {x: 0, y: 0}, Point {x: 0, y: 0}, Point {x: 0, y: 0}, Point {x: 0, y: 0}];
        // take four coordinates
        for i in 0..4 {
            points[i] = take_point();
        }
        if distance(&points[0], &points[1]) < distance(&points[0], &points[2]) {
            println!("{}", distance(&points[0], &points[1]));
        } else {
            println!("{}", distance(&points[0], &points[2]));
        }
    }
    
}