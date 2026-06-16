import os
from google import genai
from google.genai import errors
from dotenv import load_dotenv

def generate_readme():
    # Get the absolute path of the root/scripts/ folder
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    # Secure absolute paths relative to the script location
    log_dir = os.path.abspath(os.path.join(script_dir, "../logbook"))
    output_path = os.path.abspath(os.path.join(script_dir, "../README.md"))
    env_path = os.path.abspath(os.path.join(script_dir, "../.env"))

    # Load .env vars explicitly from the root directory path
    load_dotenv(dotenv_path=env_path)
    
    # Verify API key
    if not os.environ.get("GEMINI_API_KEY"):
        print("GEMINI_API_KEY not found.")
        print(f"Please create a '.env' file in your root folder: {os.path.dirname(script_dir)}")
        print("See '.env.example' for reference.")
        return

    # Initialize Gemini client
    try:
        client = genai.Client()
    except Exception as e:
        print(f"Initialization Error: {e}")
        return
    
    if not os.path.exists(log_dir):
        print(f"Error: Could not find the logbook folder at: {log_dir}")
        return

    # Gather and merge all logbook markdown files chronologically
    log_content = ""
    try:
        log_files = sorted([f for f in os.listdir(log_dir) if f.endswith(".md")])
    except Exception as e:
        print(f"Error reading logbook directory: {e}")
        return
    
    if not log_files:
        print(f"Error: No markdown (.md) logs found inside the logbook folder at: {log_dir}")
        return

    for filename in log_files:
        file_path = os.path.join(log_dir, filename)
        with open(file_path, "r", encoding="utf-8") as f:
            log_content += f"\n\n--- Start of Log: {filename} ---\n"
            log_content += f.read()
            log_content += f"\n--- End of Log: {filename} ---\n"

    # Craft defensive structure instructions for the model
    prompt = (
        f"You are an expert technical writer. Synthesize the provided project logbooks "
        f"into a clean, professional README.md file using a strict structural template.\n\n"
        f"CRITICAL LAYOUT RULE:\n"
        f"Evaluate the project data for each section requested below. If a section is not "
        f"relevant to the project, lacks sufficient tracking context, or has absolutely "
        f"no data provided in the logs, OMIT that section entirely. Do not generate empty headers, "
        f"and do not write generic filler or placeholder text.\n\n"
        f"REQUESTED README SECTIONS (Include ONLY if sufficient data exists):\n"
        f"1. Badges: Include status pills, project size indicators, or license badges if declared in metadata.\n"
        f"2. Project Title\n"
        f"3. High-Level Overview\n"
        f"4. Table of Contents: Dynamically map only the sections that survive the data filter.\n"
        f"5. System Architecture\n"
        f"6. Tech Stack and Dependencies\n"
        f"7. Features: Synthesize free-form notes to extract a clean, logically categorized list of working capabilities.\n"
        f"8. Prerequisites / System Requirements\n"
        f"9. Step-by-Step Installation / Setup\n"
        f"10. Configuration & Environment Variables\n"
        f"11. Quick Start / Usage Examples\n"
        f"12. Hardware Pinout / Interconnect Map\n"
        f"13. License Declaration\n\n"
        f"OUTPUT CONSTRAINT:\n"
        f"- Output ONLY raw, valid markdown syntax.\n"
        f"- Do not wrap the entire response inside markdown code blocks (do not start with ```markdown).\n"
        f"- Ignore conversational developer ramblings, active debugging logs, or temporary errors.\n\n"
        f"Project Logs Data:\n{log_content}"
    )

    print(f"Found {len(log_files)} log file(s). Processing with Gemini AI...")

    # Call the free Gemini Flash model
    try:
        response = client.models.generate_content(
            model='gemini-2.5-flash',
            contents=prompt,
        )
        
        # Save the generated text directly to your root README.md
        with open(output_path, "w", encoding="utf-8") as f:
            f.write(response.text)
            
        print(f"Success! README.md has been generated and updated at: {output_path}")
        
    except errors.APIError as e:
        print(f"Gemini API Error: {e}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

if __name__ == "__main__":
    generate_readme()
