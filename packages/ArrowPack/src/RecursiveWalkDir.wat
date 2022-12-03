(module
  (import "env" "readdir" (func $readdir (param i32 i32) (result i32)))
  (import "env" "stat" (func $stat (param i32 i32) (result i32)))
  (import "env" "malloc" (func $malloc (param i32) (result i32)))
  (import "env" "free" (func $free (param i32)))
  (import "env" "memcpy" (func $memcpy (param i32 i32 i32)))

(memory 1)

(func $get_string (param $ptr i32) (result i32)
  (local $len i32)
  (local $str_ptr i32)

  (set_local $len (i32.load (get_local $ptr)))
  (set_local $str_ptr (call $malloc (i32.add (get_local $len) (i32.const 1))))
  (call $memcpy (get_local $str_ptr) (i32.add (get_local $ptr) (i32.const 4)) (get_local $len))
  (i32.store (i32.add (get_local $str_ptr) (get_local $len)) (i32.const 0))

  (get_local $str_ptr)
)

(func $free_string_list (param $list_ptr i32)
  (local $len i32)
  (local $i i32)

  (set_local $len (i32.load (get_local $list_ptr)))
  (set_local $i (i32.const 0))
  (block $loop
    (br_if $loop (i32.ge_u (get_local $i) (get_local $len)))
    (call $free (i32.load (i32.add (get_local $list_ptr) (i32.mul (get_local $i) (i32.const 4)))))
    (set_local $i (i32.add (get_local $i) (i32.const 1)))
    (br $loop)
  )
)
(func $walk_dir (param $dir_path i32) (result i32)
  (local $dir_entries i32)
  (local $stat_buf i32)
  (local $files_ptr i32)
  (local $files_len i32)
  (local $dirs_ptr i32)
  (local $dirs_len i32)

  (set_local $files_ptr (call $malloc (i32.const 4)))
  (set_local $files_len (i32.const 0))
  (set_local $dirs_ptr (call $malloc (i32.const 4)))
  (set_local $dirs_len (i32.const 0))

  (set_local $dir_entries (call $readdir (i32.const 0) (get_local $dir_path)))
  (block $loop
    (br_if $loop (i32.eqz (get_local $dir_entries)))
    (set_local $stat_buf (call $stat (i32.const 0) (get_local $dir_path)))

    (if (i32.and (i32.load (get_local $stat_buf)) (i32.const 16))
      (block $is_directory
        (set_local $dirs_ptr (call $malloc (i32.const 4)))
        (set_local $dirs_len (i32.add (get_local $dirs_len) (i32.const 1)))
        (i32.store (get_local $dirs_ptr) (call $get_string (i32.const 0) (get_local $dir_path)))
        (call $free_string_list (i32.const 0) (get_local $dir_entries))
        (call $walk_dir (get_local $dir_path))
      )
    )
    (else
      (block $is_file
        (set_local $files_ptr (call $malloc (i32.const 4)))
        (set_local $files_len (i32.add (get_local $files_len) (i32.const 1)))
        (i32.store (get_local $files_ptr) (call $get_string (i32.const 0) (get_local $dir_path)))
        (call $free_string_list (i32.const 0) (get_local $dir_entries))
      )
    )

    (set_local $dir_entries (call $readdir (i32.const 0) (get_local $dir_path)))
    (br $loop)
  )

  (call $free (get_local $dir_path))

  (i32.const 0)
))
