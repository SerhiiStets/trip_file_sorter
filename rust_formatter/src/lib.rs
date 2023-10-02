use pyo3::prelude::*;
use pyo3::types::PyDict;
use std::fs;

// impl IntoPy<PyObject> for File{
//     fn into_py(self, py: Python) -> PyObject {
//         let gil = pyo3::Python::acquire_gil();
//         let py = gil.python();

//         let mut dict = PyDict::new(py);
//         dict.set_item("file_name", self.file_name).unwrap();
//         dict.set_item("file_path", self.file_path).unwrap();
//         dict.set_item("file_format", self.file_format).unwrap();
//         dict.set_item("new_name", self.new_name).unwrap();

//         dict.into()
//     }
// }

#[pyclass]
struct File {
    file_name: String,
    file_path: String,
    file_format: String,
    new_name: String,
}

#[pymethods]
impl File {
    #[new]
    fn new(file_name: String, file_path: String, file_format: String, new_name: String) -> Self {
        File {
            file_name,
            file_path,
            file_format,
            new_name,
        }
    }

    #[getter]
    fn file_name(&self) -> &str {
        &self.file_name
    }
    #[getter]
    fn file_path(&self) -> &str {
        &self.file_path
    }
    #[getter]
    fn file_format(&self) -> &str {
        &self.file_format
    }
    #[getter]
    fn new_name(&self) -> &str {
        &self.new_name
    }
}

#[pyfunction]
fn get_sorted_files(dir_path: &str, start_date: u32, end_date: u32) -> Vec<File> {
    let mut file_infos: Vec<File> = Vec::new();
    if let Ok(entries) = fs::read_dir(dir_path) {
        for entry in entries {
            if let Ok(entry) = entry {
                let path = entry.path();
                if let Some(name) = path.file_stem() {
                    if let Some(extension) = path.extension() {
                        println!("File Name: {}", name.to_string_lossy());
                        println!("File Format: {}", extension.to_string_lossy());
                        let file_info = File {
                            file_name: String::new(),
                            file_path: path.display().to_string(),
                            file_format: String::new(),
                            new_name: String::new(),
                        };
                        file_infos.push(file_info);
                    }
                }
            }
        }
    } else {
        std::io::Error::new(std::io::ErrorKind::Other, "Unable to read directory");
    }
    file_infos
}

/// A Python module implemented in Rust.
#[pymodule]
fn rust_formatter(_py: Python, m: &PyModule) -> PyResult<()> {
    m.add_class::<File>()?;
    m.add_function(wrap_pyfunction!(get_sorted_files, m)?)?;
    Ok(())
}
