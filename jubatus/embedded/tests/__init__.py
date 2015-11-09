def test_native_import():
    from jubatus.embedded import get_core_version
    assert get_core_version() == '0.2.4'
