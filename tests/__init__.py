def test_native_import():
    from jubatus.embedded import get_core_version
    vers = [int(x) for x in get_core_version().split('.')]
    assert vers[0] == 0 and (vers[1] > 2 or (vers[1] == 2 and vers[2] >= 5))
