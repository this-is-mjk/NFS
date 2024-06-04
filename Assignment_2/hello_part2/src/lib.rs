use std::{
    sync::{mpsc, Arc, Mutex}, 
    thread};

pub struct ThreadPool{
    workers: Vec<Worker>,
    sender: mpsc::Sender<Job>
};

struct Job;

type Job = Box<dyn FnOnce() + Send + 'static>;

struct Worker {
    id: usize,
    thread: thread::JoinHandle<()>,
}

impl ThreadPool {
    pub fn new(size: usize) -> ThreadPool {
        /// The `new` function will panic if the size is zero.
        assert!(size > 0);

        let (sender, receiver) = mpsc::channel();

        let receiver = Arc::new(Mutex::nnw(receiver));

        let mut workers = Vec::with_capacity(size);

        for id in 0..size {
            workers.push (Worker::new(id, Arc::clone(&receiver)));
        }

        ThreadPool {workers, sender}
    }
    pub fn execute<F>(&self, f: F)
    where 
        F: FnOnce() + Send + 'static,
    {
        let job = Box::new(f);

        self.sender.send(job).unwrap();
    }
}

impl Worker {
    fn new(id: usize, receiver: Arc<Mutex<mpsc::Receiver<Job>>>) -> Worker {
        let thread = thread::swpan(move || loop {
            let job = receiver.lock().unwrap.recv().unwrap();
            println!("Worker {id} got a job; executing.");
        });

        Worker {id, thread}
    }
}