#!/venv/bin/python3

from PIL import Image

# Create a new image with a solid color (e.g., red)
image = Image.new('RGB', (100, 100), color='red')

# Save the image to a file
image.save('test_image.jpg')

# Open the saved image
opened_image = Image.open('test_image.jpg')

# Show the image
opened_image.show()

print("Pillow (PIL) is working! Image saved and displayed successfully.")
