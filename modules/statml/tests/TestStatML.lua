local StatML = import "@StatML"

function TestCases.load_data()
    StatML.parse_file("modules/dungenerate-data/aptitudes.yaml")
end