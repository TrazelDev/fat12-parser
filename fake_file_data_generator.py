from faker import Faker
fake = Faker()

with open('bin/fake_data.txt', 'w') as f:
    for _ in range(100):
        f.write(fake.paragraph() + '\n')
