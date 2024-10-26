use std::io;
use std::collections::HashMap;

fn get_number() -> u32 {
    // Declare string
    let mut input = String::new();

    // Input string
    io::stdin().read_line(&mut input).unwrap();

    // Return number
    input.trim().parse().unwrap()
}
fn take_len_rem() -> Point {
    // Declare string
    let mut input = String::new();

    // Input string
    io::stdin().read_line(&mut input).unwrap();

    // Split string
    let mut iter = input.trim().split_whitespace();

    // Return point
    return Point {
        len: iter.next().unwrap().parse().unwrap(),
        rem: iter.next().unwrap().parse().unwrap(),
    };
}
struct Point {
    len: u32,
    rem: u32,
}
fn main() {
    // Get number of test cases
    let t = get_number();

    // Iterate over test cases
    for _ in 0..t {
        // Get length and to be removed
        let point = take_len_rem();
        // Declare string
        let mut input = String::new();
        // Input string
        io::stdin().read_line(&mut input).unwrap();
        do_work(input, &point);
    }
}

fn do_work(input_string: String, point: &Point) {
    let letter_counts: HashMap<char, i32> =
        input_string
            .trim()
            .to_lowercase()
            .chars()
            .fold(HashMap::new(), |mut map, c| {
                *map.entry(c).or_insert(0) += 1;
                map
            });
    let mut count = 0;
    for (_key, value) in letter_counts {
        if value % 2 != 0 {
            count += 1;
        }
    }
    if count > point.rem + 1 {
        println!("NO");
    } else if (point.rem - count) % 2 == 0{
        println!("YES");
    }
    else if (point.len - point.rem) % 2 == 0 {
        println!("YES");
    } 
    else if (point.len - point.rem) % 2 != 0 && count - point.rem == 1 {
        println!("YES");
    } 
    else {
        println!("NO");
    }
}
