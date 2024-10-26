use std::io;

fn get_number() -> u32 {
    // Declare string
    let mut input = String::new();

    // Input string
    io::stdin().read_line(&mut input).unwrap();

    // Return number
    input.trim().parse().unwrap()
}
fn check_div(number: u32, differnce: u32) -> bool{
    // println!("number: {}\ndiffernece: {}\n", number, differnce);
    let mut count = 0;
    let mut divisor =  vec![0;4];
    for i in (1..(number+1)).rev(){
        if number % i == 0{
            divisor[count] = i;
            count += 1;
            // println!("{}: {}\n", count, divisor[count-1]);
        }
        if count >= 2 && divisor[count-2] - divisor[count-1] < differnce {
            return false;
        }
        if count == 4 {
            // println!("count: 4\n");
            break;
        }
    }
    if count < 4 {
        false
    } else if divisor[2] - divisor[3] >= differnce {
        // println!("divisor[3]: {}\ndivisor[2]: {}\ndiff: {}", divisor[3], divisor[2], differnce);
        true
    } else {
        false
    }
}

fn main() {
    // Get number of rounds
    let rounds = get_number();
    for _ in 0..rounds {
        let differnce = get_number();
        let mut number = 6;
        loop {
            if check_div(number, differnce){
                println!("{}", number);
                break;
            }
            number += 1;
        }
    }
}